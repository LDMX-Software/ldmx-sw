#include "Builder.hh"

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <set>
#include <algorithm>
#include <chrono>

#include "Fragment.hh"
#include "EventSummary.hh"
#include "Packing/LDMXRoRHeader.h"

using namespace eventbuilder;

void Builder::configure(framework::config::Parameters &ps) {
  if (ps.exists("verbose_parse")) {
    m_verbose_parse = ps.get<bool>("verbose_parse");
  }
    std::cerr << "[Builder] configure(): dat_file='" << m_input_file << "' output_name='" << m_output_name
                        << "' verbose_parse=" << (m_verbose_parse?"true":"false") << std::endl;
    if (ps.exists("dat_file")) {
        m_input_file = ps.get<std::string>("dat_file");
    } else {
        const char* env_input = std::getenv("EVENTBUILDER_INPUT");
        if (env_input) m_input_file = env_input;
    }
    if (ps.exists("output_name")) m_output_name = ps.get<std::string>("output_name");
}

void Builder::produce(framework::Event &event) {
    // Load input file once
    if (!m_input_loaded) {
        if (m_input_file.empty()) {
            std::cerr << "[Builder] no input file specified\n";
            abortEvent();
            return;
        }
        std::ifstream in(m_input_file, std::ios::binary);
        if (!in) {
            std::cerr << "[Builder] failed to open input file: '" << m_input_file << "'\n";
            abortEvent();
            return;
        }
        in.seekg(0, std::ios::end);
        std::streampos fsize = in.tellg();
        in.seekg(0, std::ios::beg);
        if (fsize > 0 && (fsize % 4) == 0) {
            size_t nw = static_cast<size_t>(fsize / 4);
            m_words.resize(nw);
            in.read(reinterpret_cast<char*>(m_words.data()), fsize);
            // find starts
            for (size_t i = 0; i < nw; ++i) {
                uint8_t top = static_cast<uint8_t>((m_words[i] >> 24) & 0xff);
                if (top == 0xA5) m_starts.push_back(i);
            }
            std::cerr << "[Builder] loaded file size=" << fsize << " bytes (" << nw << " words), found "
                      << m_starts.size() << " RoR starts" << std::endl;
        }
        m_input_loaded = true;
        m_start_idx = 0;
    }

    // Iterate frames until we can build and return one assembled event
    const long long coherence_window_ns = 5000000;
    while (true) {
        if (m_start_idx >= m_starts.size()) {
            // no more frames to process -> abort
            std::cerr << "[Builder] no more frames to process (start_idx=" << m_start_idx << ", starts.size="
                      << m_starts.size() << ")" << std::endl;
            abortEvent();
            return;
        }
        size_t s = m_starts[m_start_idx];
        size_t e = (m_start_idx + 1 < m_starts.size()) ? m_starts[m_start_idx + 1] : m_words.size();
        size_t frame_words = e - s;
        m_start_idx++;

        if (frame_words < 4) {
            if (m_verbose_parse) std::cerr << "[Builder] skipping short frame at index " << s << " (words=" << frame_words << ")\n";
            continue;
        }
        if (s + 1 < m_words.size()) {
            uint8_t channel = static_cast<uint8_t>((m_words[s + 1] >> 24) & 0xff);
            if (channel == 255) {
                if (m_verbose_parse) std::cerr << "[Builder] skipping YAML/config frame at index " << s << "\n";
                continue;
            }
        } else {
            if (m_verbose_parse) std::cerr << "[Builder] unexpected end of words at s+1\n";
            continue;
        }

        DataFragment fragment;
        uint64_t contrib = 0, subsys = 0, ror_ts = 0;
        bool ok = parse_ror_header(m_words, s, contrib, subsys, ror_ts);
        if (ok) {
            fragment.header.timestamp = ror_ts;
            fragment.header.subsystem_id = subsys;
            if (m_verbose_parse) std::cerr << "[Builder] parsed RoR header at word idx " << s << " subsys=" << subsys << " ts=" << ror_ts << std::endl;
        } else {
            if (m_verbose_parse) std::cerr << "[Builder] RoR header parse failed at idx " << s << ", trying packing subsystem parse" << std::endl;
            std::vector<uint32_t> pkt_words;
            uint64_t pkt_subsys = 0;
            uint64_t pkt_event = 0;
            if (!try_parse_packing_subsystem(m_words, s, pkt_words, pkt_subsys, pkt_event)) continue;
            fragment.header.subsystem_id = pkt_subsys;
            fragment.header.timestamp = pkt_event * 1000000000ULL;
            frame_words = pkt_words.size();
        }

        fragment.payload.resize(frame_words * 4);
        std::memcpy(fragment.payload.data(), reinterpret_cast<char*>(m_words.data() + s), frame_words * 4);
        if (m_verbose_parse) std::cerr << "[Builder] adding fragment subsys=" << fragment.header.subsystem_id
                        << " ts=" << fragment.header.timestamp << " bytes=" << fragment.payload.size() << std::endl;
        m_event_buffer.add_fragment(std::move(fragment));

        // try to build an event
        std::vector<DataFragment> assembled_event_fragments;
        long long current_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        bool built = m_event_buffer.try_build_event(current_time, coherence_window_ns, assembled_event_fragments, false);
        if (built && !assembled_event_fragments.empty()) {
            // assign a new event id and assemble payload
            ++m_event_id;
            PhysicsEventData final_event = assemble_payload(assembled_event_fragments);

            // add to framework event
            event.add(m_output_name, final_event);

            // set RoR timestamp as event header param
            event.getEventHeader().setIntParameter("RoR Timestamp", static_cast<long long>(final_event.timestamp));

            // persist binary event and append a summary line (retain behavior from the background merger)
            write_event_binary(final_event, "events.bin");
            std::cerr << "[Builder] assembled event id=" << m_event_id << " timestamp=" << final_event.timestamp
                      << " fragments=" << assembled_event_fragments.size() << " systems=" << final_event.systems_readout.size() << std::endl;
            EventSummary summary;
            summary.event_id = m_event_id;
            summary.timestamp_ns = static_cast<uint64_t>(final_event.timestamp);
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
            summary.error_flags = has_unknown ? 2 : 0;
            std::ofstream ofs("event_summaries.txt", std::ios::app);
            if (ofs) ofs << summary.to_csv() << '\n';
            return;
        }
        // otherwise loop to process more frames
    }
}

bool Builder::parse_ror_header(const std::vector<uint32_t>& words, size_t index,
                                    uint64_t& contrib, uint64_t& subsys, uint64_t& timestamp) {
    const size_t total_bytes = words.size() * 4;
    const size_t byte_offset = index * 4;
    if (byte_offset + packing::LDMXRoRHeader::SIZE > total_bytes) return false;

    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(words.data()) + byte_offset;
    uint8_t sentinel = bytes[3];
    if (sentinel != 0xA5) return false;
    uint8_t version = bytes[0];
    uint8_t subsystem = bytes[1];
    uint8_t contributor = bytes[2];
    uint64_t raw_ts = 0;
    for (int i = 0; i < 8; ++i) raw_ts |= (static_cast<uint64_t>(bytes[8 + i]) << (8 * i));

    contrib = static_cast<uint64_t>(contributor);
    subsys = static_cast<uint64_t>(subsystem);
    uint64_t seconds_out = 0;
    if (raw_ts > 1000000000ULL && raw_ts < 3000000000ULL) seconds_out = raw_ts;
    else if (raw_ts > 1000000000000ULL) seconds_out = raw_ts / 1000000000ULL;
    else seconds_out = raw_ts;
    timestamp = seconds_out * 1000000000ULL;
    if (m_verbose_parse) {
        std::cerr << "[VERBOSE] RoR hdr idx=" << index << " ver=" << (int)version
                  << " subsys=0x" << std::hex << (int)subsystem << std::dec
                  << " contrib=" << (int)contributor
                  << " raw_ts=" << raw_ts << " -> s=" << seconds_out << std::endl;
    }
    return true;
}

bool Builder::try_parse_packing_subsystem(const std::vector<uint32_t>& words, size_t s,
                                               std::vector<uint32_t>& out_words, uint64_t& subsys,
                                               uint64_t& event_number_out) {
    if (s + 3 > words.size()) return false;
    uint32_t head = words[s];
    uint16_t id = static_cast<uint16_t>((head >> 16) & 0xFFFF);
    uint32_t len = (head >> 1) & 0x7FFFu;
    size_t needed = 1 + 1 + static_cast<size_t>(len) + 1;
    if (s + needed > words.size()) return false;
    if (len > 1024 * 1024) return false;
    out_words.clear();
    out_words.reserve(needed);
    for (size_t i = s; i < s + needed; ++i) out_words.push_back(words[i]);
    subsys = static_cast<uint64_t>(id);
    event_number_out = static_cast<uint64_t>(words[s + 1]);
    if (m_verbose_parse) {
        std::cerr << "[VERBOSE] Packing Subsystem pkt idx=" << s << " head=0x" << std::hex << head << std::dec
                  << " id=" << id << " len=" << len << " event=" << event_number_out << std::endl;
    }
    return true;
}

PhysicsEventData Builder::assemble_payload(const std::vector<DataFragment>& fragments) {
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

void Builder::write_event_binary(const PhysicsEventData &ev, const std::string &path) {
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
        if (psz) ofs.write(b.data.data(), static_cast<std::streamsize>(psz));
    }
    ofs.flush();
}

// Register producer with the framework factory
DECLARE_PRODUCER(eventbuilder::Builder)
