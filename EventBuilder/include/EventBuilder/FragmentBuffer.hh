// FragmentBuffer.hh (revised)
#ifndef FRAGMENTBUFFER_H
#define FRAGMENTBUFFER_H
#pragma once
#include <map>
#include <vector>
#include <mutex>
#include <chrono>
#include "Fragment.hh"
#include <set>

namespace eventbuilder {

class FragmentBuffer {
public:
    using Timestamp = long long;

    void add_fragment(DataFragment&& fragment) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_fragments[fragment.header.timestamp].push_back(std::move(fragment));
    }

    bool has_expired_fragments(Timestamp reference_time, long long coherence_window_ns) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_fragments.empty()) {
            return false;
        }
        auto it_oldest = m_fragments.begin();
        return it_oldest->first < reference_time - coherence_window_ns;
    }

    bool try_build_event(Timestamp reference_time, long long coherence_window_ns, std::vector<DataFragment>& built_fragments, bool force_assemble = false) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_fragments.empty()) return false;

        Timestamp window_ref_time = force_assemble ? m_fragments.begin()->first : reference_time;

        auto it_begin = m_fragments.lower_bound(window_ref_time - coherence_window_ns);
        auto it_end = m_fragments.upper_bound(window_ref_time + coherence_window_ns);

        if (it_begin == it_end) return false;

        std::set<uint64_t> subsystems_found;
        std::vector<Timestamp> timestamps_in_window;

        for (auto it = it_begin; it != it_end; ++it) {
            timestamps_in_window.push_back(it->first);
            for (const auto& frag : it->second) {
                subsystems_found.insert(frag.header.subsystem_id);
            }
        }

        if (!force_assemble && subsystems_found.size() < 3) {
            return false;
        }

        for (Timestamp ts : timestamps_in_window) {
            for (auto& frag : m_fragments[ts]) {
                built_fragments.push_back(std::move(frag));
            }
            m_fragments.erase(ts);
        }
        return true;
    }

private:
    std::map<Timestamp, std::vector<DataFragment>> m_fragments;
    std::mutex m_mutex;
};

} // namespace eventbuilder

#endif // FRAGMENTBUFFER_H
