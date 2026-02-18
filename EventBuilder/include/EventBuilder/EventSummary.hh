// EventSummary.hh - simple event summary structure for event builder
#ifndef EVENT_SUMMARY_H
#define EVENT_SUMMARY_H

#include <cstdint>
#include <vector>
#include <string>
#include <sstream>

namespace eventbuilder {

struct EventSummary {
    uint64_t event_id = 0;
    uint64_t timestamp_ns = 0;
    uint32_t nsystems = 0;
    std::vector<uint64_t> system_ids;
    uint64_t payload_size = 0;
    uint32_t error_flags = 0;

    // CSV format: eventID,timestamp_ns,nsystems,systemid1;systemid2;...,payload_size,errorflags
    std::string to_csv() const {
        std::ostringstream oss;
        oss << event_id << ',' << timestamp_ns << ',' << nsystems << ',';
        for (size_t i = 0; i < system_ids.size(); ++i) {
            if (i) oss << ';';
            oss << system_ids[i];
        }
        oss << ',' << payload_size << ',' << error_flags;
        return oss.str();
    }
};

} // namespace eventbuilder

#endif // EVENT_SUMMARY_H
