#pragma once
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "SimCore/Event/SimParticle.h"
#include "Tracking/Event/Measurement.h"
#include "Tracking/Event/Track.h"
#include "Tracking/Reco/TruthMatchingTool.h"

namespace tracking {
namespace sim {

class TruthMatchingTool {
 public:
  struct TruthInfo {
    int trackID;
    double truthProb;
    int pdgID;
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

  ~TruthMatchingTool() = default;

  TruthInfo TruthMatch(const ldmx::Track& trk);
  TruthInfo Evaluate(
      const std::unordered_map<unsigned int, unsigned int>& trk_trackIDs,
      int n_meas);
  TruthInfo TruthMatch(const std::vector<ldmx::Measurement>& vmeas);

  bool configured() { return configured_; }

 private:
  std::map<int, ldmx::SimParticle> map_;
  std::vector<ldmx::Measurement> measurements_;
  bool debug_{false};
  std::shared_ptr<tracking::sim::TruthMatchingTool> truthMatchingTool = nullptr;
  bool configured_{false};
};

}  // namespace sim
}  // namespace tracking
