/**
 * @file CascadeHistoryStore.h
 * @brief Thread-local storage for cascade histories during simulation
 */

#ifndef SIMCORE_BERTINI_CASCADEHISTORYSTORE_H
#define SIMCORE_BERTINI_CASCADEHISTORYSTORE_H

#include <map>

#include "SimCore/Event/CascadeHistory.h"

namespace simcore {
namespace bertini {

/**
 * @class CascadeHistoryStore
 * @brief Thread-local singleton for cascade histories during simulation
 *
 * Stores histories keyed by initiating photon track ID. Thread-local for
 * multi-threaded Geant4. Call clear() at event start, extractHistories()
 * at event end.
 */
class CascadeHistoryStore {
 public:
  static CascadeHistoryStore& getInstance();

  void clear() { histories_.clear(); }
  void addHistory(int trackId, ldmx::CascadeHistory history);
  bool empty() const { return histories_.empty(); }
  size_t size() const { return histories_.size(); }

  const std::map<int, ldmx::CascadeHistory>& getHistories() const {
    return histories_;
  }

  /** Moves histories out and clears store */
  std::map<int, ldmx::CascadeHistory> extractHistories();

  bool hasHistory(int trackId) const {
    return histories_.find(trackId) != histories_.end();
  }

  const ldmx::CascadeHistory* getHistory(int trackId) const;

 private:
  CascadeHistoryStore() = default;
  ~CascadeHistoryStore() = default;
  CascadeHistoryStore(const CascadeHistoryStore&) = delete;
  CascadeHistoryStore& operator=(const CascadeHistoryStore&) = delete;

  std::map<int, ldmx::CascadeHistory> histories_;
};

}  // namespace bertini
}  // namespace simcore

#endif  // SIMCORE_BERTINI_CASCADEHISTORYSTORE_H
