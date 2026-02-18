#include <iostream>
#include <thread>
#include <chrono>
#include <memory>
#include <atomic>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <span>

#include "Fragment.hh"
#include "FragmentBuffer.hh"
#include "PhysicsEventData.hh"
#include "EventSummary.hh"
#include "Packing/RawDataFile/SubsystemPacket.h"
#include "Packing/LDMXRoRHeader.h"

#include <csignal> // Required for signal handling
#include <set>
#include <algorithm>
#include <fstream>

#include <mutex>

#include "EventBuilder.hh"

int main() {
    eventbuilder::EventBuilder eb;
    return eb.run();
}

// Try to parse a Packing::rawdatafile::SubsystemPacket at word index 's'.
// On success, fills subsys and returns true. Also returns the payload words in out_words.
static bool try_parse_packing_subsystem(const std::vector<uint32_t>& words, size_t s,
                                       std::vector<uint32_t>& out_words, uint64_t& subsys,
                                       uint64_t& event_number_out) {
    // Need at least header (word), event (word), CRC (tail)
    if (s + 3 > words.size()) return false;
    uint32_t head = words[s];
    // id: high 16 bits
    uint16_t id = static_cast<uint16_t>((head >> 16) & 0xFFFF);
    // len: bits [1..15]
    uint32_t len = (head >> 1) & 0x7FFFu;

    // Only accept electronics subsystem ids in the expected 1..5 range
    //if (id < 1 || id > 8) return false;

    // Compute expected end index: head + event + len data + tail
    size_t needed = 1 + 1 + static_cast<size_t>(len) + 1; // head,event,data...,crc
    if (s + needed > words.size()) return false;

    // Basic sanity: len shouldn't be absurdly large
    if (len > 1024 * 1024) return false;

    // Extract the data words for the subsystem packet (head..tail inclusive)
    out_words.clear();
    out_words.reserve(needed);
    for (size_t i = s; i < s + needed; ++i) out_words.push_back(words[i]);

    subsys = static_cast<uint64_t>(id);
    // event number is second word
    event_number_out = static_cast<uint64_t>(words[s + 1]);
    if (verbose_parse) {
        std::cerr << "[VERBOSE] Packing Subsystem pkt idx=" << s << " head=0x" << std::hex << head << std::dec
                  << " id=" << id << " len=" << len << " event=" << event_number_out << std::endl;
    }
    return true;
}

std::string subsystem_id_to_string(uint64_t id) {
    switch (id) {
        case 0x01: return "TDAQ_TRIGGER_SUBSYSTEM";
        case 0x02: return "TS_DAQ_SUBSYSTEM";
        case 0x03: return "TS_TRIGGER_SUBSYSTEM";
        case 0x04: return "TRACKER_DAQ_SUBSYSTEM";
        case 0x05: return "ECAL_DAQ_SUBSYSTEM";
        case 0x06: return "ECAL_TRIGGER_SUBSYSTEM";
        case 0x07: return "HCAL_DAQ_SUBSYSTEM";
        case 0x08: return "HCAL_TRIGGER_SUBSYSTEM";
        case 0xAA: return "GENERIC_SUBSYSTEM";
        default: return "Unknown";
    }
}

// Function to gather and assemble fragments into a complete event payload
PhysicsEventData assemble_payload(const std::vector<DataFragment>& fragments) {
    PhysicsEventData event_data;
    if (fragments.empty()) return event_data;

    // Use the timestamp and event ID from the first fragment as the reference
    event_data.event_id = event_id; // global counter is used by the merger
    event_data.timestamp = fragments.front().header.timestamp;

    // Convert DataFragment -> GenericDataBlock (keep payload encoded)
    for (const auto &fragment : fragments) {
        GenericDataBlock g;
        g.subsystem_id = fragment.header.subsystem_id;
        g.timestamp_ns = fragment.header.timestamp;
        g.data = fragment.payload; // copy raw bytes
        // prefer trailer checksum when present; fall back to computed CRC
        if (fragment.trailer.checksum != 0) g.checksum = fragment.trailer.checksum;
        else g.checksum = crc32(fragment.payload);
        event_data.blocks.push_back(std::move(g));
        event_data.systems_readout.push_back(fragment.header.subsystem_id);
    }

    return event_data;
}

// Append a binary, generic event record to disk for downstream DAQ
// Format (all integer fields little-endian native):
//  - uint64_t event_id
//  - uint64_t timestamp_ns
//  - uint32_t nblocks
// For each block:
//  - uint64_t subsystem_id
//  - uint64_t block_timestamp_ns
//  - uint32_t payload_size
//  - uint32_t checksum
//  - payload bytes (payload_size)
static std::mutex g_out_mutex;
void write_event_binary(const PhysicsEventData &ev, const std::string &path="events_out.dat") {
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
        if (psz) ofs.write(b.data.data(), static_cast<std::streamsize>(psz));
    }
    ofs.flush();
}



// Function that runs in a separate thread to merge fragments into full events
void eventMergerThread(FragmentBuffer& buffer) {
    // Group fragments within a 5ms coherence window by timestamp
    long long coherence_window_ns = 5000000;

    while (app_running) {
        std::vector<DataFragment> assembled_event_fragments;
        long long current_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();

        bool built = buffer.try_build_event(current_time, coherence_window_ns, assembled_event_fragments, false);

        if (built && !assembled_event_fragments.empty()) {
            uint64_t ev_ts_ns = static_cast<uint64_t>(assembled_event_fragments.front().header.timestamp);
            std::cout << "\n[MERGER] Built event with timestamp: " << ev_ts_ns
                      << " fragments=" << assembled_event_fragments.size() << std::endl;
            std::cout << "[MERGER] (s=" << (ev_ts_ns / 1000000000ULL) << ") Subsystems:";
            for (const auto &f : assembled_event_fragments) std::cout << ' ' << f.header.subsystem_id;
            std::cout << std::endl;
            PhysicsEventData final_event = assemble_payload(assembled_event_fragments);
            // Write binary event record for downstream DAQ
            write_event_binary(final_event);

            // Build and write summary record
            EventSummary summary;
            summary.event_id = ++event_id;
            summary.timestamp_ns = ev_ts_ns;
            std::set<uint64_t> unique_sys;
            uint64_t total_payload = 0;
            bool has_unknown = false;
            for (const auto &f : assembled_event_fragments) {
                unique_sys.insert(f.header.subsystem_id);
                if (f.header.subsystem_id == 255) has_unknown = true;
                total_payload += static_cast<uint64_t>(f.payload.size());
            }
            summary.nsystems = static_cast<uint32_t>(unique_sys.size());
            summary.system_ids.assign(unique_sys.begin(), unique_sys.end());
            summary.payload_size = total_payload;
            summary.error_flags = 0; // 0 => complete
            if (has_unknown) summary.error_flags |= 2; // bit 1 => unknown subsystem present

            std::ofstream ofs("event_summaries.txt", std::ios::app);
            if (ofs) ofs << summary.to_csv() << '\n';
        } else if (buffer.has_expired_fragments(current_time, coherence_window_ns)) {
            buffer.try_build_event(current_time, coherence_window_ns, assembled_event_fragments, true);
            if (!assembled_event_fragments.empty()) {
                uint64_t forced_ns = static_cast<uint64_t>(assembled_event_fragments.front().header.timestamp);
                std::cout << "[MERGER] Forced assembly of partial event with timestamp: "
                          << forced_ns
                          << " fragments=" << assembled_event_fragments.size() << " (Timeout)" << std::endl;
                std::cout << "[MERGER] (s=" << (forced_ns / 1000000000ULL) << ") Subsystems:";
                for (const auto &f : assembled_event_fragments) std::cout << ' ' << f.header.subsystem_id;
                std::cout << std::endl;

                // Write a summary for forced/partial event
                EventSummary summary;
                summary.event_id = ++event_id;
                summary.timestamp_ns = forced_ns;
                std::set<uint64_t> unique_sys;
                uint64_t total_payload = 0;
                bool has_unknown = false;
                for (const auto &f : assembled_event_fragments) {
                    unique_sys.insert(f.header.subsystem_id);
                    if (f.header.subsystem_id == 255) has_unknown = true;
                    total_payload += static_cast<uint64_t>(f.payload.size());
                }
                summary.nsystems = static_cast<uint32_t>(unique_sys.size());
                summary.system_ids.assign(unique_sys.begin(), unique_sys.end());
                summary.payload_size = total_payload;
                summary.error_flags = 1; // 1 => forced/partial
                if (has_unknown) summary.error_flags |= 2; // bit 1 => unknown subsystem present
                std::ofstream ofs("event_summaries.txt", std::ios::app);
                if (ofs) ofs << summary.to_csv() << '\n';
                // Also write the partial event payload in the binary stream
                PhysicsEventData final_event = assemble_payload(assembled_event_fragments);
                write_event_binary(final_event);
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

// Signal handler function
void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received. Shutting down gracefully..." << std::endl;
    app_running = false; // Safely sets the flag to false
}

int main() {
    // Register signal handler for SIGINT (Ctrl+C)
    signal(SIGINT, signalHandler);
    std::cout << "Starting Event Builder (file-stream mode)..." << std::endl;

    // Build shared buffer and start merger thread
    FragmentBuffer event_buffer;
    std::thread merger_thread(eventMergerThread, std::ref(event_buffer));

    // Input file (default to the example file in the project directory)
    std::string input_file = "Run_061_20251211_145655.dat";

    // Allow overriding from command line by setting an environment variable or future argv support
    const char* env_input = std::getenv("EVENTBUILDER_INPUT");
    if (env_input) input_file = env_input;
    const char* env_verbose = std::getenv("EVENTBUILDER_VERBOSE_PARSE");
    if (env_verbose && std::string(env_verbose) == "1") verbose_parse = true;

    try {
        std::ifstream in(input_file, std::ios::binary);
        if (!in) {
            std::cerr << "Failed to open input file: " << input_file << std::endl;
            app_running = false;
        } else {
            std::cout << "Reading input from: " << input_file << std::endl;

            // Try word-aligned pflib-style parsing: read entire file as 32-bit words and
            // locate LDMX RoR headers (0xA5 sentinel in the top byte of a word). This avoids
            // trusting an external payload_size field that may be malformed.
            in.seekg(0, std::ios::end);
            std::streampos fsize = in.tellg();
            in.seekg(0, std::ios::beg);

            if (fsize > 0 && (fsize % 4) == 0) {
                size_t nw = static_cast<size_t>(fsize / 4);
                std::vector<uint32_t> words(nw);
                in.read(reinterpret_cast<char*>(words.data()), fsize);

                // Find indices where sentinel == 0xA5 in the top byte (byte offset 3)
                std::vector<size_t> starts;
                for (size_t i = 0; i < nw; ++i) {
                    uint8_t top = static_cast<uint8_t>((words[i] >> 24) & 0xff);
                    if (top == 0xA5) starts.push_back(i);
                }

                if (starts.empty()) {
                    // No RoR headers found; fall back to legacy framed parsing below
                    in.clear();
                    in.seekg(0, std::ios::beg);
                } else {
                    // For each start, slice until next start (or EOF) and create fragment
                    for (size_t si = 0; si < starts.size() && app_running; ++si) {
                        size_t s = starts[si];
                        size_t e = (si + 1 < starts.size()) ? starts[si + 1] : nw;
                        size_t frame_words = e - s;
                        if (frame_words < 4) continue; // need at least header+timestamp words

                        // Skip YAML/config frames: the frame header channel byte is at byte offset 7
                        // (top byte of words[s+1] in the word-aligned buffer). Channel==255 => YAML/config.
                        if (s + 1 < nw) {
                            uint8_t channel = static_cast<uint8_t>((words[s + 1] >> 24) & 0xff);
                            if (channel == 255) continue;
                        } else {
                            continue;
                        }

                        DataFragment fragment;

                        // Try to parse RoR header using our local parser (handles endianness)
                        uint64_t contrib = 0, subsys = 0, ror_ts = 0;

                        bool ok = parse_ror_header(words, s, contrib, subsys, ror_ts);
                        if (ok) {
                            fragment.header.timestamp = ror_ts;

                            // assign parsed subsystem id directly (use top-level Packing API instead of local packet ctor)
                            fragment.header.subsystem_id = subsys;
                        } else {
                            // Attempt to parse as Packing SubsystemPacket layout
                            std::vector<uint32_t> pkt_words;
                            uint64_t pkt_subsys = 0;
                            uint64_t pkt_event = 0;
                            if (!try_parse_packing_subsystem(words, s, pkt_words, pkt_subsys, pkt_event)) continue;

                            fragment.header.subsystem_id = pkt_subsys;
                            // use event number as a timestamp proxy (seconds -> ns)
                            fragment.header.timestamp = pkt_event * 1000000000ULL;
                            // payload already comes from pkt_words below
                            // overwrite frame_words to match the parsed packet length
                            frame_words = pkt_words.size();
                        }

                        // copy raw bytes for this frame into payload (preserve original byte order)
                        fragment.payload.resize(frame_words * 4);
                        std::memcpy(fragment.payload.data(), reinterpret_cast<char*>(words.data() + s), frame_words * 4);

                        uint64_t fts_ns = static_cast<uint64_t>(fragment.header.timestamp);
                        std::cout << "[INPUT] Added fragment: ts=" << fts_ns
                                  << " (s=" << (fts_ns / 1000000000ULL) << ")"
                                  << " subsys=" << fragment.header.subsystem_id
                                  << " bytes=" << fragment.payload.size() << std::endl;
                        event_buffer.add_fragment(std::move(fragment));
                    }

                    // Done parsing word-mode file
                    // Sleep briefly to allow merger thread to process
                    while (app_running) std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    in.close();
                }
            }

            if (!in || !app_running) {
                // either file read failed or we finished
            } else {
                // Fallback legacy parsing if not processed as word-aligned frames
                in.clear();
                in.seekg(0, std::ios::beg);
                while (app_running && in) {
                    // Read header words
                    uint64_t timestamp_raw = 0;
                    uint64_t subsystem_id = 0;
                    size_t payload_size = 0;

                    if (!in.read(reinterpret_cast<char*>(&timestamp_raw), sizeof(timestamp_raw))) break;
                    if (!in.read(reinterpret_cast<char*>(&subsystem_id), sizeof(subsystem_id))) break;
                    if (!in.read(reinterpret_cast<char*>(&payload_size), sizeof(payload_size))) break;

                    DataFragment fragment;
                    fragment.header.timestamp = timestamp_raw;
                    fragment.header.subsystem_id = subsystem_id;

                    // Protect against malformed/huge payload_size values that can cause bad_alloc
                    const size_t MAX_PAYLOAD = 50 * 1024 * 1024; // 50 MB

                    // Determine remaining bytes in file
                    std::streampos cur_pos = in.tellg();
                    in.seekg(0, std::ios::end);
                    std::streampos end_pos = in.tellg();
                    in.seekg(cur_pos);
                    size_t remaining = 0;
                    if (end_pos != -1 && cur_pos != -1 && end_pos > cur_pos) {
                        remaining = static_cast<size_t>(end_pos - cur_pos);
                    }

                    if (payload_size > remaining) {
                        std::cerr << "Warning: payload_size (" << payload_size << ") larger than remaining file bytes (" << remaining << "). Truncating." << std::endl;
                        payload_size = remaining;
                    }
                    if (payload_size > MAX_PAYLOAD) {
                        std::cerr << "Error: payload_size (" << payload_size << ") exceeds maximum allowed (" << MAX_PAYLOAD << "). Aborting." << std::endl;
                        app_running = false;
                        break;
                    }

                    try {
                        fragment.payload.resize(payload_size);
                    } catch (const std::bad_alloc& e) {
                        std::cerr << "Allocation failed for payload of size " << payload_size << ": " << e.what() << std::endl;
                        app_running = false;
                        break;
                    }

                    if (payload_size)
                        in.read(fragment.payload.data(), static_cast<std::streamsize>(payload_size));

                    // Read trailer (if present) - safe read attempt
                    FragmentTrailer trailer;
                    if (in.read(reinterpret_cast<char*>(&trailer), sizeof(trailer))) {
                        fragment.trailer = trailer;
                    }

                    // Try local RoR header parse from the payload (word-aligned)
                    if (fragment.payload.size() >= 4 && (fragment.payload.size() % 4) == 0) {
                        size_t nw = fragment.payload.size() / 4;
                        std::vector<uint32_t> frame_words(nw);
                        std::memcpy(frame_words.data(), fragment.payload.data(), fragment.payload.size());
                        uint64_t contrib = 0, subsys = 0, ror_ts = 0;
                        if (parse_ror_header(frame_words, 0, contrib, subsys, ror_ts)) {
                            if (ror_ts != 0) fragment.header.timestamp = ror_ts;
                            fragment.header.subsystem_id = subsys;
                        } else {
                            // fallback heuristic: first 8 bytes
                            uint64_t candidate = 0;
                            std::memcpy(&candidate, fragment.payload.data(), sizeof(candidate));
                            if (candidate != 0) fragment.header.timestamp = static_cast<long long>(candidate);
                        }
                        } else if (fragment.payload.size() >= sizeof(uint64_t)) {
                        uint64_t candidate = 0;
                        std::memcpy(&candidate, fragment.payload.data(), sizeof(candidate));
                        if (candidate != 0) fragment.header.timestamp = candidate;
                    }

                    uint64_t fts_ns = static_cast<uint64_t>(fragment.header.timestamp);
                    std::cout << "[INPUT] Added fragment: ts=" << fts_ns
                              << " (s=" << (fts_ns / 1000000000ULL) << ")"
                              << " subsys=" << fragment.header.subsystem_id
                              << " bytes=" << fragment.payload.size() << std::endl;
                    event_buffer.add_fragment(std::move(fragment));

                    // Small sleep to simulate streaming; adjust or remove for faster processing
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Fatal Error reading input file: " << e.what() << std::endl;
        app_running = false;
    }

    // Cleanup phase: Wait for the merger thread to finish its current loop iteration
    if (merger_thread.joinable()) {
        merger_thread.join();
    }
    std::cout << "Application finished cleanly." << std::endl;
    return 0;
}
