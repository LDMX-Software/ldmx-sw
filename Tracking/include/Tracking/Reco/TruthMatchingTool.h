#pragma once
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "SimCore/Event/SimParticle.h"
#include "Tracking/Event/Measurement.h"
#include "Tracking/Event/NewTrack.h"
#include "Tracking/Event/Track.h"
#include "Tracking/Reco/TruthMatchingTool.h"

namespace tracking {
namespace sim {

class TruthMatchingTool {
 public:
  struct TruthInfo {
    int track_id_;
    double truth_prob_;
    int pdg_id_;
  };

  TruthMatchingTool() = default;

  /**
   * Constructor.
   *
   * @param particleMap The map of all the simulated particles in the event.
   * @param measurements All the measurements in the event.
   */

  TruthMatchingTool(const std::map<int, ldmx::SimParticle>& particleMap,
                    const std::vector<ldmx::Measurement>& measurements) {
    setup(particleMap, measurements);
  };

  void setup(const std::map<int, ldmx::SimParticle>& particleMap,
             const std::vector<ldmx::Measurement>& measurements) {
    map_ = particleMap;
    measurements_ = measurements;
    configured_ = true;
  }

  /**
   * Destructor.
   */
  virtual ~TruthMatchingTool() = default;

  TruthInfo truthMatch(const ldmx::Track& trk);
  TruthInfo truthMatch(const ldmx::NewTrack& trk);
  TruthInfo evaluate(
      const std::unordered_map<unsigned int, unsigned int>& trk_trackIDs,
      int n_meas);
  TruthInfo truthMatch(const std::vector<ldmx::Measurement>& vmeas);

  bool configured() { return configured_; }

 private:
  std::map<int, ldmx::SimParticle> map_;
  std::vector<ldmx::Measurement> measurements_;
  bool debug_{false};
  std::shared_ptr<tracking::sim::TruthMatchingTool> truth_matching_tool_ =
      nullptr;
  bool configured_{false};
};

}  // namespace sim
}  // namespace tracking
