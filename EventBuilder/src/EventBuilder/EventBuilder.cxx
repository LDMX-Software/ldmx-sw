#include "EventBuilder/EventBuilder.hh"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <cstring>
#include <set>
#include <algorithm>
#include <chrono>

#include "EventBuilder/Fragment.hh"
#include "EventBuilder/Event/PhysicsEventData.hh"
#include "EventBuilder/Event/GenericDataBlock.hh"
#include "Framework/EventSummary.h"
#include "Packing/LDMXRoRHeader.h"
#include "Packing/RogueFrameHeader.h"
#include "Packing/RawDataFile/SubsystemPacket.h"

using namespace eventbuilder;

// Global flag to track if performance CSV header has been written
static bool g_perf_csv_header_written = false;

// Helper function to write performance metrics to CSV
void write_performance_metric(
    unsigned int event_id,
    double build_time_ms,
    double cum_events_per_sec,
    double cum_mb_per_sec,
    double window_events_per_sec,
    double window_mb_per_sec,
    uint64_t total_events,
    uint64_t total_bytes) {
    
    std::ofstream perf_csv("event_performance.csv", std::ios::app);
    if (!perf_csv) return;
    
    // Write header on first call
    if (!g_perf_csv_header_written) {
        perf_csv << "event_id,event_build_time_ms,cum_events_per_sec,cum_mb_per_sec,"
                 << "window_events_per_sec,window_mb_per_sec,total_events,total_bytes_mb\n";
        g_perf_csv_header_written = true;
    }
    
    // Write data with consistent formatting
    double total_mb = total_bytes / (1024.0 * 1024.0);
    perf_csv << event_id << ","
             << std::fixed << std::setprecision(3) << build_time_ms << ","
             << std::fixed << std::setprecision(2) << cum_events_per_sec << ","
             << std::fixed << std::setprecision(3) << cum_mb_per_sec << ","
             << std::fixed << std::setprecision(2) << window_events_per_sec << ","
             << std::fixed << std::setprecision(3) << window_mb_per_sec << ","
             << total_events << ","
             << std::fixed << std::setprecision(2) << total_mb << "\n";
    perf_csv.flush();
}

void EventBuilder::configure(framework::config::Parameters &ps) {
  if (ps.exists("verbose_parse")) {
    m_verbose_parse = ps.get<bool>("verbose_parse");
  }
  if (ps.exists("dat_file")) {
    m_input_file = ps.get<std::string>("dat_file");
  } else {
    const char* env_input = std::getenv("EVENTBUILDER_INPUT");
    if (env_input) m_input_file = env_input;
  }
  if (ps.exists("output_name")) m_output_name = ps.get<std::string>("output_name");
  
  std::cerr << "[EventBuilder] configure(): dat_file='" << m_input_file << "' output_name='" << m_output_name
                      << "' verbose_parse=" << (m_verbose_parse?"true":"false") << std::endl;
  std::cerr.flush();
  
  if (!m_input_file.empty()) {
    std::cerr << "[EventBuilder] configure(): opening file '" << m_input_file << "'" << std::endl;
    std::cerr.flush();
    m_reader.open(m_input_file);
    if (!m_reader) {
      std::cerr << "[EventBuilder] ERROR: failed to open input file: '" << m_input_file << "'\n";
    } else {
      std::cerr << "[EventBuilder] configure(): file opened successfully" << std::endl;
    }
  } else {
    std::cerr << "[EventBuilder] ERROR: no input file specified\n";
  }
  std::cerr.flush();

  // Initialize performance tracking
  m_start_time = std::chrono::steady_clock::now();
  m_total_bytes_read = 0;
  m_total_events_built = 0;
  m_events_since_last_report = 0;
  
  // Initialize windowed metrics for real-time DAQ monitoring
  m_window_start_time = m_start_time;
  m_window_events_count = 0;
  m_window_bytes_read = 0;
}

void EventBuilder::produce(framework::Event &event) {
    static int produce_call_count = 0;
    produce_call_count++;
    std::cerr << "[EventBuilder] produce() called, count=" << produce_call_count << std::endl;
    std::cerr<< "[EventBuilder] verbose_parse=" << (m_verbose_parse ? "true" : "false") << std::endl;
    if (m_verbose_parse || produce_call_count <= 3) {
        std::cerr << "[EventBuilder] produce() call #" << produce_call_count << std::endl;
    }

    // Start timing for this event
    m_event_start_time = std::chrono::steady_clock::now();

    // Read ONE event per produce() call
    // Loop through frames in the file until we form a complete event
    const long long coherence_window_ns = 5000000;  // 5 ms window for collecting fragments from same physics event
    
    // Track errors encountered during event assembly
    uint32_t current_event_errors = 0;
    
    // Helper to convert subsystem ID to name
    auto subsystem_id_to_name = [](uint64_t id) -> std::string {
        switch(id) {
            case 1: return "TDAQTrigger";
            case 2: return "TSDaq";
            case 3: return "TSTrigger";
            case 4: return "TrackerDaq";
            case 5: return "EcalDaq";
            case 6: return "EcalTrigger";
            case 7: return "HcalDaq";
            case 8: return "HcalTrigger";
            case 9: return "Generic";
            default: return "Subsystem" + std::to_string(id);
        }
    };
    
    while (m_reader && !m_reader.eof()) {
        // Try to read a RogueFrameHeader
        packing::RogueFrameHeader frame_header;
        frame_header.read(m_reader);
        
        // Store location of end-of-frame for potential recovery
        const long long frame_end = m_reader.tell() + frame_header.size();
        
        // Check if this is a data frame (channel 0) and not YAML
        if (frame_header.channel() != 0 || frame_header.probablyYaml()) {
            // Skip this frame
            if (m_verbose_parse) std::cerr << "[EventBuilder] skipping non-data frame (channel=" << frame_header.channel() << ")\n";
            m_reader.seek(frame_end);
            continue;
        }
        
        // We have a valid data frame - try to parse it
        DataFragment fragment{{}, {}, {.checksum = 0}};
        bool parsed_ok = false;
        
        // Try RoR format first
        packing::LDMXRoRHeader ror_header;
        int pos_before_ror = m_reader.tell();
        
        // Attempt to read as RoR header
        try {
            ror_header.read(m_reader);
            // Successfully parsed RoR header
            fragment.header.subsystem_id = static_cast<uint64_t>(ror_header.subsystem());
            fragment.header.timestamp = ror_header.timestamp();
            
            if (m_verbose_parse) {
                std::cerr << "[EventBuilder] parsed RoR header subsys=" << (int)ror_header.subsystem()
                          << " contrib=" << (int)ror_header.contributor() 
                          << " ts=" << ror_header.timestamp() << std::endl;
            }
            
            // Read remaining frame payload after RoR header
            std::vector<uint8_t> payload_data;
            long long payload_size = frame_end - m_reader.tell();
            if (payload_size > 0) {
                payload_data.resize(payload_size);
                m_reader.read(reinterpret_cast<char*>(payload_data.data()), payload_size);
            }
            fragment.payload = std::move(payload_data);
            parsed_ok = true;
        } catch (...) {
            // RoR parse failed, try packing subsystem format
            if (m_verbose_parse) std::cerr << "[EventBuilder] RoR header parse failed, trying packing subsystem format\n";
            current_event_errors |= ldmx::EventSummary::ERROR_PARSE_FAILURE;
            m_reader.seek(pos_before_ror);
        }
        
        if (!parsed_ok) {
            // Try packing subsystem format
            packing::rawdatafile::SubsystemPacket pkt;
            try {
                pkt.read(m_reader);
                fragment.header.subsystem_id = static_cast<uint64_t>(pkt.id());
                fragment.header.timestamp = static_cast<uint64_t>(pkt.header()[1]) * 1000000000ULL;  // event number to ns
                
                // Convert packet data to payload bytes
                const auto& data = pkt.data();
                fragment.payload.resize(data.size() * 4);
                std::memcpy(fragment.payload.data(), reinterpret_cast<const char*>(data.data()), data.size() * 4);
                
                if (m_verbose_parse) {
                    std::cerr << "[EventBuilder] parsed packing subsystem pkt subsys=" << pkt.id()
                              << " data_size=" << data.size() << std::endl;
                }
                parsed_ok = true;
            } catch (...) {
                // Both parse attempts failed, skip this frame
                if (m_verbose_parse) std::cerr << "[EventBuilder] failed to parse as either format, skipping frame\n";
                current_event_errors |= ldmx::EventSummary::ERROR_PARSE_FAILURE;
                m_reader.seek(frame_end);
                continue;
            }
        }
        
        if (!parsed_ok) {
            m_reader.seek(frame_end);
            continue;
        }

        if (m_verbose_parse) std::cerr << "[EventBuilder] adding fragment subsys=" << fragment.header.subsystem_id
                        << " ts=" << fragment.header.timestamp << " bytes=" << fragment.payload.size() << std::endl;
        
        // Check if this fragment is outside the coherence window of the current event batch
        // If so, it means we should finalize the previous event before adding this one
        long long fragment_ts = fragment.header.timestamp;
        long long buffer_ref_time = m_event_buffer.get_reference_time();
        
        if (buffer_ref_time > 0 && (fragment_ts < buffer_ref_time - coherence_window_ns || 
                                     fragment_ts > buffer_ref_time + coherence_window_ns)) {
            // This fragment is outside the current window - try to build the previous event
            std::vector<DataFragment> assembled_event_fragments;
            if (m_event_buffer.try_build_event(coherence_window_ns, assembled_event_fragments) && 
                !assembled_event_fragments.empty()) {
                // Output the complete event and return
                ++m_event_id;
                // Add each subsystem's raw data as vector<uint8_t> directly
                uint64_t event_timestamp = 0;
                for (const auto &frag : assembled_event_fragments) {
                    std::string subsys_name = subsystem_id_to_name(frag.header.subsystem_id);
                    std::vector<uint8_t> payload_bytes(frag.payload.begin(), frag.payload.end());
                    event.add(subsys_name, payload_bytes);
                    if (event_timestamp == 0) {
                        event_timestamp = frag.header.timestamp;
                    }
                }
                event.getEventHeader().setIntParameter("RoR Timestamp", static_cast<long long>(event_timestamp));
                // Still create PhysicsEventData for binary output file
                PhysicsEventData final_event = assemble_payload(assembled_event_fragments);
                write_event_binary(final_event, "events.bin");
                std::cerr << "[EventBuilder] assembled event id=" << m_event_id << " timestamp=" << final_event.timestamp
                          << " fragments=" << assembled_event_fragments.size() << " systems=" << final_event.systems_readout.size() << std::endl;
                
                ldmx::EventSummary summary;
                summary.setEventId(m_event_id);
                summary.setTimestampNs(static_cast<uint64_t>(final_event.timestamp));
                std::set<uint64_t> unique_sys;
                uint64_t total_payload = 0;
                for (const auto &f : assembled_event_fragments) {
                    unique_sys.insert(f.header.subsystem_id);
                    total_payload += f.payload.size();
                }
                // Check for duplicate subsystems
                if (unique_sys.size() != assembled_event_fragments.size()) {
                    current_event_errors |= ldmx::EventSummary::ERROR_DUPLICATE_SUBSYSTEM;
                }
                summary.setNSystems(static_cast<uint32_t>(unique_sys.size()));
                summary.setSystemIds(std::vector<uint64_t>(unique_sys.begin(), unique_sys.end()));
                summary.setPayloadSize(total_payload);
                summary.setErrorFlags(current_event_errors);
                event.add("EventSummary", summary);
                
                // Update performance metrics
                m_total_bytes_read += total_payload;
                m_total_events_built++;
                m_events_since_last_report++;
                auto now = std::chrono::steady_clock::now();
                auto event_build_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_event_start_time);
                auto total_elapsed_s = std::chrono::duration_cast<std::chrono::seconds>(now - m_start_time);
                
                double events_per_sec = (total_elapsed_s.count() > 0) ? (double)m_total_events_built / total_elapsed_s.count() : 0.0;
                double mb_per_sec = (total_elapsed_s.count() > 0) ? (double)m_total_bytes_read / (1024.0 * 1024.0) / total_elapsed_s.count() : 0.0;
                
                // Update windowed metrics
                m_window_events_count++;
                m_window_bytes_read += total_payload;
                auto window_elapsed_s = std::chrono::duration_cast<std::chrono::seconds>(now - m_window_start_time);
                
                // Reset window if we've processed WINDOW_SIZE events
                double window_events_per_sec = 0.0;
                double window_mb_per_sec = 0.0;
                if (m_window_events_count >= WINDOW_SIZE) {
                    double window_time_sec = std::max(1.0, (double)window_elapsed_s.count());
                    window_events_per_sec = (double)m_window_events_count / window_time_sec;
                    window_mb_per_sec = (double)m_window_bytes_read / (1024.0 * 1024.0) / window_time_sec;
                    
                    // Reset window
                    m_window_start_time = now;
                    m_window_events_count = 0;
                    m_window_bytes_read = 0;
                }
                
                // Write performance metrics to CSV
                auto event_build_time_ms_val = event_build_time_ms.count();
                write_performance_metric(m_event_id, static_cast<double>(event_build_time_ms_val), 
                                        events_per_sec, mb_per_sec, 
                                        window_events_per_sec, window_mb_per_sec,
                                        m_total_events_built, m_total_bytes_read);
                
                if (m_verbose_parse || m_events_since_last_report % 100 == 0) {
                    std::cerr << "[EventBuilder] Performance (Cumulative): "
                              << "events_per_sec=" << std::fixed << std::setprecision(2) << events_per_sec << ", "
                              << "mb_per_sec=" << std::fixed << std::setprecision(3) << mb_per_sec << " | "
                              << "(Window-100): "
                              << "events_per_sec=" << std::fixed << std::setprecision(2) << window_events_per_sec << ", "
                              << "mb_per_sec=" << std::fixed << std::setprecision(3) << window_mb_per_sec << " | "
                              << "build_time=" << event_build_time_ms.count() << "ms" << std::endl;
                }
                
                current_event_errors = 0;  // Reset for next event
                return;  // Return the event to framework
            }
        }
        
        // Add fragment to buffer (may start a new event batch if buffer was empty)
        m_event_buffer.add_fragment(std::move(fragment));
        
        if (m_verbose_parse) std::cerr << "[EventBuilder] frame added to buffer, searching for more frames...\n";
    }

    // Reached EOF - flush any remaining events in the buffer
    if (m_verbose_parse) std::cerr << "[EventBuilder] reached EOF, flushing remaining events\n";
    
    std::vector<DataFragment> assembled_event_fragments;
    while (m_event_buffer.try_build_event(coherence_window_ns, assembled_event_fragments)) {
        if (assembled_event_fragments.empty()) break;
        
        // Mark truncated events (those flushed at EOF)
        current_event_errors |= ldmx::EventSummary::ERROR_TRUNCATED_EVENT;
        
        ++m_event_id;
        // Add each subsystem's raw data as vector<uint8_t> directly
        uint64_t event_timestamp = 0;
        for (const auto &frag : assembled_event_fragments) {
            std::string subsys_name = subsystem_id_to_name(frag.header.subsystem_id);
            std::vector<uint8_t> payload_bytes(frag.payload.begin(), frag.payload.end());
            event.add(subsys_name, payload_bytes);
            if (event_timestamp == 0) {
                event_timestamp = frag.header.timestamp;
            }
        }
        event.getEventHeader().setIntParameter("RoR Timestamp", static_cast<long long>(event_timestamp));
        // Still create PhysicsEventData for binary output file
        PhysicsEventData final_event = assemble_payload(assembled_event_fragments);
        write_event_binary(final_event, "events.bin");
        std::cerr << "[EventBuilder] assembled event id=" << m_event_id << " timestamp=" << final_event.timestamp
                  << " fragments=" << assembled_event_fragments.size() << " systems=" << final_event.systems_readout.size() << std::endl;
        
        ldmx::EventSummary summary;
        summary.setEventId(m_event_id);
        summary.setTimestampNs(static_cast<uint64_t>(final_event.timestamp));
        std::set<uint64_t> unique_sys;
        uint64_t total_payload = 0;
        for (const auto &f : assembled_event_fragments) {
            unique_sys.insert(f.header.subsystem_id);
            total_payload += f.payload.size();
        }
        // Check for duplicate subsystems
        if (unique_sys.size() != assembled_event_fragments.size()) {
            current_event_errors |= ldmx::EventSummary::ERROR_DUPLICATE_SUBSYSTEM;
        }
        summary.setNSystems(static_cast<uint32_t>(unique_sys.size()));
        summary.setSystemIds(std::vector<uint64_t>(unique_sys.begin(), unique_sys.end()));
        summary.setPayloadSize(total_payload);
        summary.setErrorFlags(current_event_errors);
        event.add("EventSummary", summary);
        
        // Update performance metrics
        m_total_bytes_read += total_payload;
        m_total_events_built++;
        m_events_since_last_report++;
        auto now = std::chrono::steady_clock::now();
        auto event_build_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_event_start_time);
        auto total_elapsed_s = std::chrono::duration_cast<std::chrono::seconds>(now - m_start_time);
        
        double events_per_sec = (total_elapsed_s.count() > 0) ? (double)m_total_events_built / total_elapsed_s.count() : 0.0;
        double mb_per_sec = (total_elapsed_s.count() > 0) ? (double)m_total_bytes_read / (1024.0 * 1024.0) / total_elapsed_s.count() : 0.0;
        
        // Update windowed metrics
        m_window_events_count++;
        m_window_bytes_read += total_payload;
        auto window_elapsed_s = std::chrono::duration_cast<std::chrono::seconds>(now - m_window_start_time);
        
        // Calculate window metrics (for final event, may not have full window)
        double window_events_per_sec = 0.0;
        double window_mb_per_sec = 0.0;
        if (m_window_events_count >= WINDOW_SIZE || window_elapsed_s.count() > 0) {
            double window_time_sec = std::max(1.0, (double)window_elapsed_s.count());
            window_events_per_sec = (double)m_window_events_count / window_time_sec;
            window_mb_per_sec = (double)m_window_bytes_read / (1024.0 * 1024.0) / window_time_sec;
        }
        
        // Write performance metrics to CSV
        auto event_build_time_ms_val = event_build_time_ms.count();
        write_performance_metric(m_event_id, static_cast<double>(event_build_time_ms_val), 
                                events_per_sec, mb_per_sec,
                                window_events_per_sec, window_mb_per_sec,
                                m_total_events_built, m_total_bytes_read);
        
        if (m_verbose_parse || m_events_since_last_report % 100 == 0) {
            std::cerr << "[EventBuilder] Performance (Cumulative): "
                      << "events_per_sec=" << std::fixed << std::setprecision(2) << events_per_sec << ", "
                      << "mb_per_sec=" << std::fixed << std::setprecision(3) << mb_per_sec << " | "
                      << "(Window-100): "
                      << "events_per_sec=" << std::fixed << std::setprecision(2) << window_events_per_sec << ", "
                      << "mb_per_sec=" << std::fixed << std::setprecision(3) << window_mb_per_sec << " | "
                      << "build_time=" << event_build_time_ms.count() << "ms" << std::endl;
        }
        
        current_event_errors = 0;  // Reset for next event
        
        assembled_event_fragments.clear();
        return;
    }
    
    // No more events - print final summary statistics
    auto final_time = std::chrono::steady_clock::now();
    auto total_time_s = std::chrono::duration_cast<std::chrono::seconds>(final_time - m_start_time);
    double final_events_per_sec = (total_time_s.count() > 0) ? (double)m_total_events_built / total_time_s.count() : 0.0;
    double final_mb_per_sec = (total_time_s.count() > 0) ? (double)m_total_bytes_read / (1024.0 * 1024.0) / total_time_s.count() : 0.0;
    
    std::cerr << "\n[EventBuilder] ===== FINAL STATISTICS =====\n"
              << "[EventBuilder] Total events built: " << m_total_events_built << "\n"
              << "[EventBuilder] Total bytes read: " << m_total_bytes_read / (1024.0 * 1024.0) << " MB\n"
              << "[EventBuilder] Total time: " << total_time_s.count() << " seconds\n"
              << "[EventBuilder] Average events/sec: " << final_events_per_sec << "\n"
              << "[EventBuilder] Average MB/sec: " << final_mb_per_sec << "\n"
              << "[EventBuilder] ============================\n" << std::endl;
    
    abortEvent();
}

PhysicsEventData EventBuilder::assemble_payload(const std::vector<DataFragment>& fragments) {
    PhysicsEventData event_data;
    if (fragments.empty()) return event_data;
    event_data.event_id = m_event_id;
    event_data.timestamp = fragments.front().header.timestamp;
    for (const auto &fragment : fragments) {
        GenericDataBlock g;
        g.subsystem_id = fragment.header.subsystem_id;
        g.timestamp_ns = fragment.header.timestamp;
        g.data = fragment.payload;
        if (fragment.trailer.checksum != 0) g.checksum = fragment.trailer.checksum;
        else g.checksum = crc32(fragment.payload);
        event_data.blocks.push_back(std::move(g));
        event_data.systems_readout.push_back(fragment.header.subsystem_id);
    }
    return event_data;
}

void EventBuilder::write_event_binary(const PhysicsEventData &ev, const std::string &path) {
    static std::mutex g_out_mutex;
    std::lock_guard<std::mutex> lg(g_out_mutex);
    std::ofstream ofs(path, std::ios::binary | std::ios::app);
    if (!ofs) return;
    uint64_t event_id_u = static_cast<uint64_t>(ev.event_id);
    uint64_t ts = static_cast<uint64_t>(ev.timestamp);
    uint32_t nblocks = static_cast<uint32_t>(ev.blocks.size());
    ofs.write(reinterpret_cast<const char*>(&event_id_u), sizeof(event_id_u));
    ofs.write(reinterpret_cast<const char*>(&ts), sizeof(ts));
    ofs.write(reinterpret_cast<const char*>(&nblocks), sizeof(nblocks));
    for (const auto &b : ev.blocks) {
        uint64_t sid = b.subsystem_id;
        uint64_t bts = b.timestamp_ns;
        uint32_t psz = static_cast<uint32_t>(b.data.size());
        uint32_t csum = b.checksum;
        ofs.write(reinterpret_cast<const char*>(&sid), sizeof(sid));
        ofs.write(reinterpret_cast<const char*>(&bts), sizeof(bts));
        ofs.write(reinterpret_cast<const char*>(&psz), sizeof(psz));
        ofs.write(reinterpret_cast<const char*>(&csum), sizeof(csum));
        if (psz) ofs.write(reinterpret_cast<const char*>(b.data.data()), static_cast<std::streamsize>(psz));
    }
    ofs.flush();
}

// Register producer with the framework factory
DECLARE_PRODUCER(eventbuilder::EventBuilder)
