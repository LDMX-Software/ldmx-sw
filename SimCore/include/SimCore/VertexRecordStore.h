/**
 * @file VertexRecordStore.h
 * @brief Thread-local storage for recorded interaction vertices during
 * simulation
 */

#ifndef SIMCORE_VERTEXRECORDSTORE_H
#define SIMCORE_VERTEXRECORDSTORE_H

#include <vector>

#include "SimCore/Event/VertexInfo.h"

namespace simcore {

/**
 * @class VertexRecordStore
 * @brief Thread-local singleton for interaction vertices recorded by user
 * actions
 *
 * UserActions (which have no access to framework::Event) push recorded
 * interaction vertices here; the Simulator producer extracts them at the end
 * of the event and saves them to the output file as "VertexRecords".
 * Thread-local for multi-threaded Geant4. Call clear() at event start,
 * extractVertices() at event end.
 */
class VertexRecordStore {
public:
  static VertexRecordStore &getInstance();

  void clear() { vertices_.clear(); }
  void addVertex(ldmx::VertexRecord vertex) {
    vertices_.push_back(std::move(vertex));
  }
  bool empty() const { return vertices_.empty(); }
  size_t size() const { return vertices_.size(); }

  /** Moves vertices out and clears store */
  std::vector<ldmx::VertexRecord> extractVertices();

private:
  VertexRecordStore() = default;
  ~VertexRecordStore() = default;
  VertexRecordStore(const VertexRecordStore &) = delete;
  VertexRecordStore &operator=(const VertexRecordStore &) = delete;

  std::vector<ldmx::VertexRecord> vertices_;
};

} // namespace simcore

#endif // SIMCORE_VERTEXRECORDSTORE_H
