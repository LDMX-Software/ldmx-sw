/**
 * @file VertexRecordStore.cxx
 */

#include "SimCore/VertexRecordStore.h"

namespace simcore {

VertexRecordStore &VertexRecordStore::getInstance() {
  // Meyer's singleton, thread_local for MT Geant4
  thread_local VertexRecordStore instance;
  return instance;
}

std::vector<ldmx::VertexRecord> VertexRecordStore::extractVertices() {
  std::vector<ldmx::VertexRecord> result = std::move(vertices_);
  vertices_.clear();
  return result;
}

} // namespace simcore
