/**
 * @file CascadeHistoryStore.cxx
 */

#include "SimCore/Bertini/CascadeHistoryStore.h"

namespace simcore {
namespace bertini {

CascadeHistoryStore& CascadeHistoryStore::getInstance() {
  // Meyer's singleton, thread_local for MT Geant4
  thread_local CascadeHistoryStore instance;
  return instance;
}

void CascadeHistoryStore::addHistory(int trackId,
                                     ldmx::CascadeHistory history) {
  // If there's already a history for this track, merge or replace
  // For now, we replace (shouldn't happen unless track ID recycling)
  histories_[trackId] = std::move(history);
}

std::map<int, ldmx::CascadeHistory> CascadeHistoryStore::extractHistories() {
  std::map<int, ldmx::CascadeHistory> result = std::move(histories_);
  histories_.clear();
  return result;
}

const ldmx::CascadeHistory* CascadeHistoryStore::getHistory(int trackId) const {
  auto it = histories_.find(trackId);
  if (it != histories_.end()) {
    return &it->second;
  }
  return nullptr;
}

}  // namespace bertini
}  // namespace simcore
