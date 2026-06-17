#ifndef TRACKING_RECO_TRACKCOMPARISONPROCESSOR_H_
#define TRACKING_RECO_TRACKCOMPARISONPROCESSOR_H_

#include <map>
#include <string>
#include <vector>

#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimParticle.h"
#include "TFile.h"
#include "TTree.h"
#include "Tracking/Event/Track.h"

namespace tracking::reco {

/**
 * Compares tracking performance between a truth-smeared hit chain and a
 * charge-digitized hit chain on a track-by-track basis.
 *
 * Both chains must run upstream in the same job, producing two separate Track
 * collections.  Tracks are matched by their truth-matched SimParticle ID
 * (Track::getTrackID()).  For each matched pair, a row is written to a flat
 * ROOT TTree and a set of quick-look TH1F histograms is filled via the
 * framework HistogramPool.
 */
class TrackComparisonProcessor : public framework::Analyzer {
 public:
  TrackComparisonProcessor(const std::string& name,
                           framework::Process& process);
  ~TrackComparisonProcessor() = default;

  void configure(framework::config::Parameters& parameters) override;
  void onProcessStart() override;
  void analyze(const framework::Event& event) override;
  void onProcessEnd() override;

 private:
  struct PairVars {
    int track_id_{-1};
    float truth_prob_s_{-1}, truth_prob_d_{-1};
    int nhits_s_{-1}, nhits_d_{-1};
    float chi2ndf_s_{-999}, chi2ndf_d_{-999};
    float d0_s_{-999}, d0_d_{-999};
    float z0_s_{-999}, z0_d_{-999};
    float phi_s_{-999}, phi_d_{-999};
    float theta_s_{-999}, theta_d_{-999};
    float qop_s_{-999}, qop_d_{-999};
    float p_s_{-999}, p_d_{-999};
    float delta_d0_{-999}, delta_z0_{-999};
    float delta_phi_{-999}, delta_theta_{-999};
    float delta_p_over_p_{-999};
    float px_s_{-999}, py_s_{-999}, pz_s_{-999};
    float px_d_{-999}, py_d_{-999}, pz_d_{-999};
    // truth (SimParticle)
    float px_t_{-999}, py_t_{-999}, pz_t_{-999};
    float p_t_{-999};
    float vx_t_{-999}, vy_t_{-999}, vz_t_{-999};
    float delta_p_over_p_s_{-999}, delta_p_over_p_d_{-999};
  };

  void setupTree(TTree* tree, PairVars& v);
  void fillPair(const ldmx::Track& smear, const ldmx::Track& digi,
                const ldmx::SimParticle& truth, PairVars& v,
                const std::string& prefix);
  void processTracker(const framework::Event& event,
                      const std::string& coll_smear,
                      const std::string& pass_smear,
                      const std::string& coll_digi,
                      const std::string& pass_digi, TTree* tree, PairVars& vars,
                      const std::string& histo_prefix);

  // configuration
  std::string trk_collection_smear_{"TaggerTracks"};
  std::string trk_collection_digi_{"TaggerDigiTracks"};
  std::string pass_name_smear_{""};
  std::string pass_name_digi_{""};
  bool do_tagger_{true};
  bool do_recoil_{false};
  std::string recoil_collection_smear_{"RecoilTracks"};
  std::string recoil_collection_digi_{"RecoilDigiTracks"};
  std::string recoil_pass_smear_{""};
  std::string recoil_pass_digi_{""};
  double min_truth_prob_{0.5};
  std::string sim_particles_pass_{""};
  std::string output_file_{"track_comparison.root"};

  // output
  TFile* file_{nullptr};
  TTree* tagger_tree_{nullptr};
  TTree* recoil_tree_{nullptr};
  PairVars tagger_vars_;
  PairVars recoil_vars_;
};

}  // namespace tracking::reco

#endif  // TRACKING_RECO_TRACKCOMPARISONPROCESSOR_H_
