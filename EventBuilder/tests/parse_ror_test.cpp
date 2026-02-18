#include <iostream>
#include <vector>
#include <cstdint>
#include <initializer_list>

static bool parse_ror_header(const std::vector<uint32_t>& words, size_t index,
                             uint64_t& contrib, uint64_t& subsys, uint64_t& timestamp) {
    if (index + 3 >= words.size()) return false;
    uint32_t w0 = words[index];
    uint8_t top = static_cast<uint8_t>((w0 >> 24) & 0xff);
    uint8_t low = static_cast<uint8_t>(w0 & 0xff);
    bool be = false;
    if (top == 0xA5) be = true;
    else if (low == 0xA5) be = false;
    else return false;

    uint32_t w2 = words[index + 2];
    uint32_t w3 = words[index + 3];

    struct Candidate { uint64_t contrib; uint64_t subsys; uint64_t ts; int score; };
    std::vector<Candidate> cands;

    auto score_ts = [](uint64_t t){
        uint64_t seconds = 0;
        if (t <= 0xFFFFFFFFULL) seconds = t;
        else if (t > 1000000000000ULL) seconds = t / 1000000000ULL;
        else seconds = t;
        if (seconds >= 1400000000ULL && seconds <= 2200000000ULL) return 15;
        if (seconds >= 1000000000ULL && seconds <= 3000000000ULL) return 8;
        return 0;
    };

    const std::initializer_list<uint64_t> allowed_subsystems = {
        0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0xAA
    };

    std::vector<int> all_shifts = {0,8,16,24};
    int sentinel_shift = be ? 24 : 0;
    for (int subs_shift : all_shifts) {
        if (subs_shift == sentinel_shift) continue;
        for (int order = 0; order < 2; ++order) {
            uint64_t c = 0, s = 0, ts = 0;
            if (be) {
                c = static_cast<uint64_t>((w0 >> 16) & 0xff);
                s = static_cast<uint64_t>((w0 >> subs_shift) & 0xff);
                if (order == 0) ts = (static_cast<uint64_t>(w3) << 32) | static_cast<uint64_t>(w2);
                else ts = (static_cast<uint64_t>(w2) << 32) | static_cast<uint64_t>(w3);
            } else {
                c = static_cast<uint64_t>((w0 >> 8) & 0xff);
                s = static_cast<uint64_t>((w0 >> subs_shift) & 0xff);
                if (order == 0) ts = (static_cast<uint64_t>(w2) << 32) | static_cast<uint64_t>(w3);
                else ts = (static_cast<uint64_t>(w3) << 32) | static_cast<uint64_t>(w2);
            }
            int sc = 0;
            bool in_allowed = false;
            for (auto a : allowed_subsystems) if (a == s) { in_allowed = true; break; }
            if (in_allowed) sc += 50;
            else if (s <= 32) sc += 5;
            sc += score_ts(ts);
            cands.push_back({c,s,ts,sc});
        }
    }

    if (cands.empty()) return false;
    auto best = cands.front();
    for (auto &cc : cands) if (cc.score > best.score) best = cc;
    if (best.score == 0) return false;

    bool chosen_allowed = false;
    for (auto a : allowed_subsystems) if (a == best.subsys) { chosen_allowed = true; break; }
    if (!chosen_allowed) {
        subsys = 255;
    } else {
        subsys = best.subsys;
    }
    contrib = best.contrib;
    uint64_t raw = best.ts;
    uint64_t seconds = 0;
    if (raw <= 0xFFFFFFFFULL) seconds = raw;
    else if (raw > 1000000000000ULL) seconds = raw / 1000000000ULL;
    else seconds = raw;
    timestamp = seconds * 1000000000ULL;
    return true;
}

int main() {
    // Construct little-endian test words matching the Python writer
    uint32_t w0 = (0x05 << 8) | 0xA5; // subsys=0x05 in byte1, sentinel low
    uint32_t w1 = 0;
    uint32_t w2 = 0;
    uint32_t w3 = 1600000000;
    std::vector<uint32_t> words = {w0,w1,w2,w3};
    uint64_t contrib=0, subsys=0, ts=0;
    bool ok = parse_ror_header(words, 0, contrib, subsys, ts);
    std::cout << "parse_ror_header returned " << ok << std::endl;
    std::cout << "contrib="<<contrib<<" subsys=0x"<<std::hex<<subsys<<std::dec<<" ts_ns="<<ts<<" (s="<<ts/1000000000ULL<<")\n";
    return 0;
}
