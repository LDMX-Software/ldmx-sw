#ifndef EVENTBUILDER_FRAGMENTBUFFER_H
#define EVENTBUILDER_FRAGMENTBUFFER_H

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
      m_event_reference_time_ = fragment.header_.timestamp_;
    }

    m_fragments_[fragment.header_.timestamp_].push_back(std::move(fragment));
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

  bool empty() const {
    std::lock_guard<std::mutex> lock(m_mutex_);
    return m_fragments_.empty();
  }

  /// whether the window around the reference time has enough subsystems
  bool isReferenceWindowComplete(long long coherence_window_ns,
                                 int min_subsystems) const {
    std::lock_guard<std::mutex> lock(m_mutex_);
    std::set<uint64_t> subsystems_found;
    for (auto it = m_fragments_.lower_bound(m_event_reference_time_ -
                                            coherence_window_ns);
         it != m_fragments_.upper_bound(m_event_reference_time_ +
                                        coherence_window_ns);
         ++it) {
      for (const auto& frag : it->second) {
        subsystems_found.insert(frag.header_.subsystem_id_);
      }
    }
    return subsystems_found.size() >= static_cast<std::size_t>(min_subsystems);
  }

  /**
   * Drop the fragments in the window around the reference time, for a
   * window that can not be completed any more, and move the reference
   * time to the oldest remaining fragment.
   *
   * @return number of fragments dropped
   */
  std::size_t dropReferenceWindow(long long coherence_window_ns) {
    std::lock_guard<std::mutex> lock(m_mutex_);
    auto it_begin =
        m_fragments_.lower_bound(m_event_reference_time_ - coherence_window_ns);
    auto it_end =
        m_fragments_.upper_bound(m_event_reference_time_ + coherence_window_ns);
    std::size_t n_dropped{0};
    for (auto it = it_begin; it != it_end; ++it) n_dropped += it->second.size();
    m_fragments_.erase(it_begin, it_end);
    if (!m_fragments_.empty()) {
      m_event_reference_time_ = m_fragments_.begin()->first;
    }
    return n_dropped;
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
        subsystems_found.insert(frag.header_.subsystem_id_);
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
