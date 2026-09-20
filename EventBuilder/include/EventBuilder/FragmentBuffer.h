#ifndef EVENTBUILDER_FRAGMENTBUFFER_H
#define EVENTBUILDER_FRAGMENTBUFFER_H

#include <chrono>
#include <map>
#include <mutex>
#include <set>
#include <vector>

#include "Fragment.h"

namespace eventbuilder {

class FragmentBuffer {
 public:
  using Timestamp = long long;

  void addFragment(DataFragment&& fragment) {
    std::lock_guard<std::mutex> lock(m_mutex_);

    // Set reference time on first fragment
    if (m_fragments_.empty()) {
      m_event_reference_time_ = fragment.header.timestamp;
    }

    m_fragments_[fragment.header.timestamp].push_back(std::move(fragment));
  }

  bool hasExpiredFragments(Timestamp reference_time,
                           long long coherence_window_ns) {
    std::lock_guard<std::mutex> lock(m_mutex_);
    if (m_fragments_.empty()) {
      return false;
    }
    auto it_oldest = m_fragments_.begin();
    return it_oldest->first < reference_time - coherence_window_ns;
  }

  Timestamp getReferenceTime() const {
    std::lock_guard<std::mutex> lock(m_mutex_);
    return m_event_reference_time_;
  }

  bool tryBuildEvent(long long coherence_window_ns, int min_subsystems,
                     std::vector<DataFragment>& built_fragments) {
    std::lock_guard<std::mutex> lock(m_mutex_);
    if (m_fragments_.empty()) return false;

    // Use the stored reference time from the first fragment in current
    // collection
    Timestamp window_ref_time = m_event_reference_time_;

    auto it_begin =
        m_fragments_.lower_bound(window_ref_time - coherence_window_ns);
    auto it_end =
        m_fragments_.upper_bound(window_ref_time + coherence_window_ns);

    if (it_begin == it_end) return false;

    std::set<uint64_t> subsystems_found;
    std::vector<Timestamp> timestamps_in_window;

    for (auto it = it_begin; it != it_end; ++it) {
      timestamps_in_window.push_back(it->first);
      for (const auto& frag : it->second) {
        subsystems_found.insert(frag.header.subsystem_id);
      }
    }

    // Require at least min_subsystems distinct subsystems in the window before
    // assembling, so we don't emit partial events. Configurable.
    if (subsystems_found.size() < static_cast<size_t>(min_subsystems)) {
      return false;
    }

    // Collect fragments and remove them from buffer
    for (Timestamp ts : timestamps_in_window) {
      for (auto& frag : m_fragments_[ts]) {
        built_fragments.push_back(std::move(frag));
      }
      m_fragments_.erase(ts);
    }

    // Reset reference time for next event
    if (!m_fragments_.empty()) {
      m_event_reference_time_ = m_fragments_.begin()->first;
    }

    return true;
  }

 private:
  std::map<Timestamp, std::vector<DataFragment>> m_fragments_;
  Timestamp m_event_reference_time_ = 0;
  mutable std::mutex m_mutex_;
};

}  // namespace eventbuilder

#endif  // FRAGMENTBUFFER_H
