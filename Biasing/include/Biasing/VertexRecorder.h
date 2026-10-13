/**
 * @file VertexRecorder.h
 * @class VertexRecorder
 * @brief Records the kinematics of particles entering and leaving each
 * interaction vertex into the event as a first-class record.
 * @author Hans Alin (LU)
 */

#ifndef BIASING_VERTEXRECORDER_H
#define BIASING_VERTEXRECORDER_H

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/Event/VertexInfo.h"
#include "SimCore/G4User/UserAction.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "Framework/Logger.h"

namespace biasing {

/**
 * Records the kinematics of the incoming particle and all outgoing products
 * of each interaction vertex into the event as a first-class record.
 */
class VertexRecorder : public simcore::UserAction {
public:
  VertexRecorder(const std::string &name,
                 framework::config::Parameters &parameters);
  ~VertexRecorder() = default;

  /// Called after every simulation step.
  void stepping(const G4Step *step) override;

  /// Reset per-event storage.
  void beginOfEventAction(const G4Event *) override;

  std::vector<simcore::TYPE> getTypes() override {
    return {simcore::TYPE::EVENT, simcore::TYPE::STEPPING};
  }

private:
  /// Only record vertices in this region (empty = anywhere).
  std::string region_;
  /// Only record vertices from this process (empty = any process).
  std::string process_;
  /// Track IDs of the vertex products, one entry per recorded vertex.
  std::vector<int> product_track_ids_;
};
} // namespace biasing

#endif
