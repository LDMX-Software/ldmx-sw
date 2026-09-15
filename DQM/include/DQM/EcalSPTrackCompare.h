#ifndef DQM_ECALSPTRACKCOMPARE_H
#define DQM_ECALSPTRACKCOMPARE_H

#include <string>

#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace dqm {

/**
 * @class EcalSPTrackCompare
 * @brief Compare the primary electron at the ECal scoring plane
 *        with ECAL tracks reconstructed by ACTS CKF.
 *
 * Finds the primary beam electron (track ID 1, PDG 11) at scoring
 * plane 31 (ECal front face), then compares its position and
 * direction with each reconstructed EcalTrack.  Fills residual
 * histograms for the closest matching track.
 */
class EcalSPTrackCompare : public framework::Analyzer {
 public:
  EcalSPTrackCompare(const std::string& name, framework::Process& process)
      : Analyzer(name, process) {}
  ~EcalSPTrackCompare() override = default;

  void configure(framework::config::Parameters& ps) override;
  void analyze(const framework::Event& event) override;

 private:
  std::string ecal_sp_coll_name_{"EcalScoringPlaneHits"};
  std::string ecal_sp_pass_name_{""};
  std::string track_collection_{"EcalTracks"};
  std::string track_pass_name_{""};
};

}  // namespace dqm

#endif
