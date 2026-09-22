#include "EventBuilder/EventBuilder.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <vector>

#include "EventBuilder/Event/GenericDataBlock.h"
#include "EventBuilder/Event/PhysicsEventData.h"
#include "EventBuilder/Fragment.h"
#include "Framework/EventSummary.h"
#include "Framework/Logger.h"
#include "Packing/LDMXRoRHeader.h"
#include "Packing/RawDataFile/SubsystemPacket.h"
#include "Packing/RogueFrameHeader.h"

using namespace eventbuilder;

// Global flag to track if performance CSV header has been written
static bool g_perf_csv_header_written = false;

// Helper function to write performance metrics to CSV
void writePerformanceMetric(unsigned int event_id, double build_time_ms,
                            double cum_events_per_sec, double cum_mb_per_sec,
                            double window_events_per_sec,
                            double window_mb_per_sec, uint64_t total_events,
                            uint64_t total_bytes) {
  std::ofstream perf_csv("event_performance.csv", std::ios::app);
  if (!perf_csv) return;

  // Write header on first call
  if (!g_perf_csv_header_written) {
    perf_csv
        << "event_id,event_build_time_ms,cum_events_per_sec,cum_mb_per_sec,"
        << "window_events_per_sec,window_mb_per_sec,total_events,total_bytes_"
           "mb\n";
    g_perf_csv_header_written = true;
  }

  // Write data with consistent formatting
  double total_mb = total_bytes / (1024.0 * 1024.0);
  perf_csv << event_id << "," << std::fixed << std::setprecision(3)
           << build_time_ms << "," << std::fixed << std::setprecision(2)
           << cum_events_per_sec << "," << std::fixed << std::setprecision(3)
           << cum_mb_per_sec << "," << std::fixed << std::setprecision(2)
           << window_events_per_sec << "," << std::fixed << std::setprecision(3)
           << window_mb_per_sec << "," << total_events << "," << std::fixed
           << std::setprecision(2) << total_mb << "\n";
  perf_csv.flush();
}

void EventBuilder::configure(framework::config::Parameters& ps) {
  if (ps.exists("verbose_parse")) {
    m_verbose_parse_ = ps.get<bool>("verbose_parse");
  }
  if (ps.exists("dat_file")) {
    m_input_file_ = ps.get<std::string>("dat_file");
  } else {
    const char* env_input = std::getenv("EVENTBUILDER_INPUT");
    if (env_input) m_input_file_ = env_input;
  }
  if (ps.exists("output_name"))
    m_output_name_ = ps.get<std::string>("output_name");
  if (ps.exists("coherence_window_ns")) {
    m_coherence_window_ns_ = ps.get<double>("coherence_window_ns");
  }
  if (ps.exists("min_subsystems")) {
    m_min_subsystems_ = ps.get<int>("min_subsystems");
  }

  ldmx_log(info) << "configure(): dat_file='" << m_input_file_
                 << "' output_name='" << m_output_name_
                 << "' verbose_parse=" << (m_verbose_parse_ ? "true" : "false")
                 << " coherence_window_ns=" << m_coherence_window_ns_
                 << " min_subsystems=" << m_min_subsystems_;

  if (!m_input_file_.empty()) {
    ldmx_log(info) << "configure(): opening file '" << m_input_file_ << "'";
    m_reader_.open(m_input_file_);
    if (!m_reader_) {
      ldmx_log(error) << "failed to open input file: '" << m_input_file_ << "'";
    } else {
      ldmx_log(info) << "configure(): file opened successfully";
    }
  } else {
    ldmx_log(error) << "no input file specified";
  }

  // Initialize performance tracking
  m_start_time_ = std::chrono::steady_clock::now();
  m_total_bytes_read_ = 0;
  m_total_events_built_ = 0;
  m_events_since_last_report_ = 0;

  // Initialize windowed metrics for real-time DAQ monitoring
  m_window_start_time_ = m_start_time_;
  m_window_events_count_ = 0;
  m_window_bytes_read_ = 0;
}

void EventBuilder::produce(framework::Event& event) {
  static int produce_call_count = 0;
  produce_call_count++;
  ldmx_log(debug) << "produce() called, count=" << produce_call_count;
  ldmx_log(debug) << "verbose_parse=" << (m_verbose_parse_ ? "true" : "false");
  if (m_verbose_parse_ || produce_call_count <= 3) {
    ldmx_log(info) << "produce() call #" << produce_call_count;
  }

  // Start timing for this event
  m_event_start_time_ = std::chrono::steady_clock::now();

  // Track errors encountered during event assembly
  uint32_t current_event_errors = 0;

  while (m_reader_ && !m_reader_.eof()) {
    // Try to read a RogueFrameHeader
    packing::RogueFrameHeader frame_header;
    frame_header.read(m_reader_);

    // Store location of end-of-frame for potential recovery
    const long long frame_end =
        static_cast<long long>(m_reader_.tell()) + frame_header.size();

    // Check if this is a data frame (channel 0) and not YAML
    if (frame_header.channel() != 0 || frame_header.probablyYaml()) {
      // Skip this frame
      if (m_verbose_parse_)
        ldmx_log(debug) << "skipping non-data frame (channel="
                        << frame_header.channel() << ")";
      m_reader_.seek(frame_end);
      continue;
    }

    // We have a valid data frame - try to parse it
    DataFragment fragment{{}, {}, {.checksum_ = 0}};
    bool parsed_ok = false;

    // Try RoR format first
    packing::LDMXRoRHeader ror_header;
    int pos_before_ror = m_reader_.tell();

    // Attempt to read as RoR header
    try {
      ror_header.read(m_reader_);
      // Successfully parsed RoR header
      fragment.header_.subsystem_id_ =
          static_cast<uint64_t>(ror_header.subsystem());
      fragment.header_.contributor_id_ =
          static_cast<uint64_t>(ror_header.contributor());
      fragment.header_.timestamp_ = ror_header.timestamp();

      if (m_verbose_parse_) {
        ldmx_log(debug) << "parsed RoR header subsys="
                        << (int)ror_header.subsystem()
                        << " contrib=" << (int)ror_header.contributor()
                        << " ts=" << ror_header.timestamp();
      }

      // Read remaining frame payload after RoR header
      std::vector<uint8_t> payload_data;
      long long payload_size = frame_end - m_reader_.tell();
      if (payload_size > 0) {
        payload_data.resize(payload_size);
        m_reader_.read(reinterpret_cast<char*>(payload_data.data()),
                       payload_size);
      }
      fragment.payload_ = std::move(payload_data);
      parsed_ok = true;
    } catch (...) {
      // RoR parse failed, try packing subsystem format
      if (m_verbose_parse_)
        ldmx_log(debug)
            << "RoR header parse failed, trying packing subsystem format";
      current_event_errors |= ldmx::EventSummary::ERROR_PARSE_FAILURE;
      m_reader_.seek(pos_before_ror);
    }

    if (!parsed_ok) {
      // Try packing subsystem format
      packing::rawdatafile::SubsystemPacket pkt;
      try {
        pkt.read(m_reader_);
        fragment.header_.subsystem_id_ = static_cast<uint64_t>(pkt.id());
        fragment.header_.contributor_id_ =
            0;  // Not available in packing subsystem format
        fragment.header_.timestamp_ = static_cast<uint64_t>(pkt.header()[1]) *
                                      1000000000ULL;  // event number to ns

        // Convert packet data to payload bytes
        const auto& data = pkt.data();
        fragment.payload_.resize(data.size() * 4);
        std::memcpy(fragment.payload_.data(),
                    reinterpret_cast<const char*>(data.data()),
                    data.size() * 4);

        if (m_verbose_parse_) {
          ldmx_log(debug) << "parsed packing subsystem pkt subsys=" << pkt.id()
                          << " data_size=" << data.size();
        }
        parsed_ok = true;
      } catch (...) {
        // Both parse attempts failed, skip this frame
        if (m_verbose_parse_)
          ldmx_log(debug) << "failed to parse as either format, skipping frame";
        current_event_errors |= ldmx::EventSummary::ERROR_PARSE_FAILURE;
        m_reader_.seek(frame_end);
        continue;
      }
    }

    if (!parsed_ok) {
      m_reader_.seek(frame_end);
      continue;
    }

    if (m_verbose_parse_)
      ldmx_log(debug) << "adding fragment subsys="
                      << fragment.header_.subsystem_id_
                      << " ts=" << fragment.header_.timestamp_
                      << " bytes=" << fragment.payload_.size();

    // Check if this fragment is outside the coherence window of the current
    // event batch If so, it means we should finalize the previous event before
    // adding this one
    long long fragment_ts = fragment.header_.timestamp_;
    long long buffer_ref_time = m_event_buffer_.getReferenceTime();

    if (buffer_ref_time > 0 &&
        (fragment_ts < buffer_ref_time - m_coherence_window_ns_ ||
         fragment_ts > buffer_ref_time + m_coherence_window_ns_)) {
      // This fragment is outside the current window - try to build the previous
      // event
      std::vector<DataFragment> assembled_event_fragments;
      if (m_event_buffer_.tryBuildEvent(m_coherence_window_ns_,
                                        m_min_subsystems_,
                                        assembled_event_fragments) &&
          !assembled_event_fragments.empty()) {
        // Output the complete event and return
        ++m_event_id_;
        // Add each subsystem's raw data as vector<uint8_t> directly
        uint64_t event_timestamp = 0;
        for (const auto& frag : assembled_event_fragments) {
          std::string subsys_name = packing::LDMXRoRHeader::getSubsystemName(
              static_cast<uint8_t>(frag.header_.subsystem_id_),
              static_cast<uint8_t>(frag.header_.contributor_id_));
          std::vector<uint8_t> payload_bytes(frag.payload_.begin(),
                                             frag.payload_.end());
          event.add(subsys_name, payload_bytes);
          if (event_timestamp == 0) {
            event_timestamp = frag.header_.timestamp_;
          }
        }
        event.getEventHeader().setIntParameter(
            "RoR Timestamp", static_cast<long long>(event_timestamp));
        // Still create PhysicsEventData for binary output file
        PhysicsEventData final_event =
            assemblePayload(assembled_event_fragments);
        writeEventBinary(final_event, "events.bin");
        ldmx_log(info) << "assembled event id=" << m_event_id_
                       << " timestamp=" << final_event.timestamp_
                       << " fragments=" << assembled_event_fragments.size()
                       << " systems=" << final_event.systems_readout_.size();

        ldmx::EventSummary summary;
        summary.setEventNumber(m_event_id_);
        summary.setTimestampNs(static_cast<uint64_t>(final_event.timestamp_));
        std::set<uint64_t> unique_sys;
        uint64_t total_payload = 0;
        for (const auto& f : assembled_event_fragments) {
          unique_sys.insert(f.header_.subsystem_id_);
          total_payload += f.payload_.size();
        }
        // Check for duplicate subsystems
        if (unique_sys.size() != assembled_event_fragments.size()) {
          current_event_errors |= ldmx::EventSummary::ERROR_DUPLICATE_SUBSYSTEM;
        }
        summary.setNSystems(static_cast<uint32_t>(unique_sys.size()));
        summary.setSystemIds(
            std::vector<uint64_t>(unique_sys.begin(), unique_sys.end()));
        summary.setPayloadSize(total_payload);
        summary.setErrorFlags(current_event_errors);
        event.add("EventSummary", summary);

        // Update performance metrics
        m_total_bytes_read_ += total_payload;
        m_total_events_built_++;
        m_events_since_last_report_++;
        std::chrono::steady_clock::time_point now =
            std::chrono::steady_clock::now();
        std::chrono::milliseconds event_build_time_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                now - m_event_start_time_);
        std::chrono::seconds total_elapsed_s =
            std::chrono::duration_cast<std::chrono::seconds>(now -
                                                             m_start_time_);

        double events_per_sec =
            (total_elapsed_s.count() > 0)
                ? static_cast<double>(m_total_events_built_) /
                      total_elapsed_s.count()
                : 0.0;
        double mb_per_sec = (total_elapsed_s.count() > 0)
                                ? static_cast<double>(m_total_bytes_read_) /
                                      (1024.0 * 1024.0) /
                                      total_elapsed_s.count()
                                : 0.0;

        // Update windowed metrics
        m_window_events_count_++;
        m_window_bytes_read_ += total_payload;
        std::chrono::seconds window_elapsed_s =
            std::chrono::duration_cast<std::chrono::seconds>(
                now - m_window_start_time_);

        // Reset window if we've processed WINDOW_SIZE events
        double window_events_per_sec = 0.0;
        double window_mb_per_sec = 0.0;
        if (m_window_events_count_ >= WINDOW_SIZE) {
          double window_time_sec =
              std::max(1.0, static_cast<double>(window_elapsed_s.count()));
          window_events_per_sec =
              static_cast<double>(m_window_events_count_) / window_time_sec;
          window_mb_per_sec = static_cast<double>(m_window_bytes_read_) /
                              (1024.0 * 1024.0) / window_time_sec;

          // Reset window
          m_window_start_time_ = now;
          m_window_events_count_ = 0;
          m_window_bytes_read_ = 0;
        }

        // Write performance metrics to CSV
        long long event_build_time_ms_val = event_build_time_ms.count();
        writePerformanceMetric(
            m_event_id_, static_cast<double>(event_build_time_ms_val),
            events_per_sec, mb_per_sec, window_events_per_sec,
            window_mb_per_sec, m_total_events_built_, m_total_bytes_read_);

        if (m_verbose_parse_ || m_events_since_last_report_ % 100 == 0) {
          ldmx_log(info) << "Performance: "
                         << "total_events=" << m_total_events_built_ << ", "
                         << "events_per_sec=" << std::fixed
                         << std::setprecision(2) << events_per_sec << ", "
                         << "mb_per_sec=" << std::fixed << std::setprecision(3)
                         << mb_per_sec;
        }

        current_event_errors = 0;  // Reset for next event
        // The fragment that tripped the window-close belongs to the NEXT
        // event, not the one we just emitted. Buffer it here (the buffer is
        // empty post-build, so this also resets the reference time) instead
        // of dropping it -- otherwise every event after the first loses its
        // triggering subsystem.
        m_event_buffer_.addFragment(std::move(fragment));
        return;  // Return the event to framework
      }
    }

    // Add fragment to buffer (may start a new event batch if buffer was empty)
    m_event_buffer_.addFragment(std::move(fragment));

    if (m_verbose_parse_)
      ldmx_log(debug) << "frame added to buffer, searching for more frames...";
  }

  // Reached EOF - flush any remaining events in the buffer
  if (m_verbose_parse_)
    ldmx_log(debug) << "reached EOF, flushing remaining events";

  std::vector<DataFragment> assembled_event_fragments;
  while (m_event_buffer_.tryBuildEvent(
      m_coherence_window_ns_, m_min_subsystems_, assembled_event_fragments)) {
    if (assembled_event_fragments.empty()) break;

    // Mark truncated events (those flushed at EOF)
    current_event_errors |= ldmx::EventSummary::ERROR_TRUNCATED_EVENT;

    ++m_event_id_;
    // Add each subsystem's raw data as vector<uint8_t> directly
    uint64_t event_timestamp = 0;
    for (const auto& frag : assembled_event_fragments) {
      std::string subsys_name = packing::LDMXRoRHeader::getSubsystemName(
          static_cast<uint8_t>(frag.header_.subsystem_id_),
          static_cast<uint8_t>(frag.header_.contributor_id_));
      std::vector<uint8_t> payload_bytes(frag.payload_.begin(),
                                         frag.payload_.end());
      event.add(subsys_name, payload_bytes);
      if (event_timestamp == 0) {
        event_timestamp = frag.header_.timestamp_;
      }
    }
    event.getEventHeader().setIntParameter(
        "RoR Timestamp", static_cast<long long>(event_timestamp));
    // Still create PhysicsEventData for binary output file
    PhysicsEventData final_event = assemblePayload(assembled_event_fragments);
    writeEventBinary(final_event, "events.bin");
    ldmx_log(info) << "assembled event id=" << m_event_id_
                   << " timestamp=" << final_event.timestamp_
                   << " fragments=" << assembled_event_fragments.size()
                   << " systems=" << final_event.systems_readout_.size();

    ldmx::EventSummary summary;
    summary.setEventNumber(m_event_id_);
    summary.setTimestampNs(static_cast<uint64_t>(final_event.timestamp_));
    std::set<uint64_t> unique_sys;
    uint64_t total_payload = 0;
    for (const auto& f : assembled_event_fragments) {
      unique_sys.insert(f.header_.subsystem_id_);
      total_payload += f.payload_.size();
    }
    // Check for duplicate subsystems
    if (unique_sys.size() != assembled_event_fragments.size()) {
      current_event_errors |= ldmx::EventSummary::ERROR_DUPLICATE_SUBSYSTEM;
    }
    summary.setNSystems(static_cast<uint32_t>(unique_sys.size()));
    summary.setSystemIds(
        std::vector<uint64_t>(unique_sys.begin(), unique_sys.end()));
    summary.setPayloadSize(total_payload);
    summary.setErrorFlags(current_event_errors);
    event.add("EventSummary", summary);

    // Update performance metrics
    m_total_bytes_read_ += total_payload;
    m_total_events_built_++;
    m_events_since_last_report_++;
    std::chrono::steady_clock::time_point now =
        std::chrono::steady_clock::now();
    std::chrono::milliseconds event_build_time_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now - m_event_start_time_);
    std::chrono::seconds total_elapsed_s =
        std::chrono::duration_cast<std::chrono::seconds>(now - m_start_time_);

    double events_per_sec = (total_elapsed_s.count() > 0)
                                ? static_cast<double>(m_total_events_built_) /
                                      total_elapsed_s.count()
                                : 0.0;
    double mb_per_sec = (total_elapsed_s.count() > 0)
                            ? static_cast<double>(m_total_bytes_read_) /
                                  (1024.0 * 1024.0) / total_elapsed_s.count()
                            : 0.0;

    // Update windowed metrics
    m_window_events_count_++;
    m_window_bytes_read_ += total_payload;
    std::chrono::seconds window_elapsed_s =
        std::chrono::duration_cast<std::chrono::seconds>(now -
                                                         m_window_start_time_);

    // Calculate window metrics (for final event, may not have full window)
    double window_events_per_sec = 0.0;
    double window_mb_per_sec = 0.0;
    if (m_window_events_count_ >= WINDOW_SIZE || window_elapsed_s.count() > 0) {
      double window_time_sec =
          std::max(1.0, static_cast<double>(window_elapsed_s.count()));
      window_events_per_sec =
          static_cast<double>(m_window_events_count_) / window_time_sec;
      window_mb_per_sec = static_cast<double>(m_window_bytes_read_) /
                          (1024.0 * 1024.0) / window_time_sec;
    }

    // Write performance metrics to CSV
    long long event_build_time_ms_val = event_build_time_ms.count();
    writePerformanceMetric(
        m_event_id_, static_cast<double>(event_build_time_ms_val),
        events_per_sec, mb_per_sec, window_events_per_sec, window_mb_per_sec,
        m_total_events_built_, m_total_bytes_read_);

    if (m_verbose_parse_ || m_events_since_last_report_ % 100 == 0) {
      ldmx_log(info) << "Performance: "
                     << "total_events=" << m_total_events_built_ << ", "
                     << "events_per_sec=" << std::fixed << std::setprecision(2)
                     << events_per_sec << ", "
                     << "mb_per_sec=" << std::fixed << std::setprecision(3)
                     << mb_per_sec;
    }

    current_event_errors = 0;  // Reset for next event

    assembled_event_fragments.clear();
    return;
  }

  // No more events - print final summary statistics
  std::chrono::steady_clock::time_point final_time =
      std::chrono::steady_clock::now();
  std::chrono::seconds total_time_s =
      std::chrono::duration_cast<std::chrono::seconds>(final_time -
                                                       m_start_time_);
  double final_events_per_sec =
      (total_time_s.count() > 0)
          ? static_cast<double>(m_total_events_built_) / total_time_s.count()
          : 0.0;
  double final_mb_per_sec = (total_time_s.count() > 0)
                                ? static_cast<double>(m_total_bytes_read_) /
                                      (1024.0 * 1024.0) / total_time_s.count()
                                : 0.0;

  ldmx_log(info) << "\n===== FINAL STATISTICS =====";
  ldmx_log(info) << "Total events built: " << m_total_events_built_;
  ldmx_log(info) << "Total bytes read: "
                 << m_total_bytes_read_ / (1024.0 * 1024.0) << " MB";
  ldmx_log(info) << "Total time: " << total_time_s.count() << " seconds";
  ldmx_log(info) << "Average throughput: "
                 << "events_per_sec=" << std::fixed << std::setprecision(2)
                 << final_events_per_sec << ", "
                 << "mb_per_sec=" << std::fixed << std::setprecision(3)
                 << final_mb_per_sec;
  ldmx_log(info) << "=============================\n";
  ldmx_log(info) << "Event building complete";

  abortEvent();
}

PhysicsEventData EventBuilder::assemblePayload(
    const std::vector<DataFragment>& fragments) {
  PhysicsEventData event_data;
  if (fragments.empty()) return event_data;
  event_data.event_id_ = m_event_id_;
  event_data.timestamp_ = fragments.front().header_.timestamp_;
  for (const auto& fragment : fragments) {
    GenericDataBlock g;
    g.subsystem_id_ = fragment.header_.subsystem_id_;
    g.timestamp_ns_ = fragment.header_.timestamp_;
    g.data_ = fragment.payload_;
    if (fragment.trailer_.checksum_ != 0)
      g.checksum_ = fragment.trailer_.checksum_;
    else
      g.checksum_ = crc32(fragment.payload_);
    event_data.blocks_.push_back(std::move(g));
    event_data.systems_readout_.push_back(fragment.header_.subsystem_id_);
  }
  return event_data;
}

void EventBuilder::writeEventBinary(const PhysicsEventData& ev,
                                    const std::string& path) {
  static std::mutex g_out_mutex;
  std::lock_guard<std::mutex> lg(g_out_mutex);
  std::ofstream ofs(path, std::ios::binary | std::ios::app);
  if (!ofs) return;
  uint64_t event_id_u = static_cast<uint64_t>(ev.event_id_);
  uint64_t ts = static_cast<uint64_t>(ev.timestamp_);
  uint32_t nblocks = static_cast<uint32_t>(ev.blocks_.size());
  ofs.write(reinterpret_cast<const char*>(&event_id_u), sizeof(event_id_u));
  ofs.write(reinterpret_cast<const char*>(&ts), sizeof(ts));
  ofs.write(reinterpret_cast<const char*>(&nblocks), sizeof(nblocks));
  for (const auto& b : ev.blocks_) {
    uint64_t sid = b.subsystem_id_;
    uint64_t bts = b.timestamp_ns_;
    uint32_t psz = static_cast<uint32_t>(b.data_.size());
    uint32_t csum = b.checksum_;
    ofs.write(reinterpret_cast<const char*>(&sid), sizeof(sid));
    ofs.write(reinterpret_cast<const char*>(&bts), sizeof(bts));
    ofs.write(reinterpret_cast<const char*>(&psz), sizeof(psz));
    ofs.write(reinterpret_cast<const char*>(&csum), sizeof(csum));
    if (psz)
      ofs.write(reinterpret_cast<const char*>(b.data_.data()),
                static_cast<std::streamsize>(psz));
  }
  ofs.flush();
}

// Register producer with the framework factory
DECLARE_PRODUCER(eventbuilder::EventBuilder)
