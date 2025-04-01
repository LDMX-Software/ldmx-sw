/**
 * @file TrackerVetoProcessor.h
 * @brief Class that flags events if they pass the tracker veto
 * @author Tamas Almos Vami, UCSB
 */

#include "Tracking/TrackerVetoProcessor.h"

namespace tracking {

void TrackerVetoProcessor::configure(framework::config::Parameters& ps) {
  tagger_track_collection_name_ =
      ps.getParameter<std::string>("tagger_track_collection", "TaggerTracks");
  recoil_track_collection_name_ =
      ps.getParameter<std::string>("recoil_track_collection", "RecoilTracks");
  input_tagger_pass_name_ =
      ps.getParameter<std::string>("input_tagger_pass_name", "");
  input_recoil_pass_name_ =
      ps.getParameter<std::string>("input_recoil_pass_name", "");
  output_collection_ = ps.getParameter<std::string>("output_collection", "");
  inverse_skim_ = ps.getParameter<bool>("inverse_skim", false);
  max_d0_ = ps.getParameter<double>("max_d0", 10.);
  max_z0_ = ps.getParameter<double>("max_z0", 40.);
  max_chi2_per_ndf_ = ps.getParameter<double>("max_chi2_per_ndf", 5.);
  min_tagger_momentum_ = ps.getParameter<double>("min_tagger_momentum", 5600.);
  min_recoil_n_ = ps.getParameter<int>("min_recoil_n", 1);
  max_recoil_n_ = ps.getParameter<int>("max_recoil_n", 1);
  min_tagger_hits_ = ps.getParameter<int>("min_tagger_hits", 4);
  min_recoil_hits_ = ps.getParameter<int>("min_recoil_hits", 4);
}

void TrackerVetoProcessor::produce(framework::Event& event) {
  auto tagger_track_collection{event.getCollection<ldmx::Track>(
      tagger_track_collection_name_, input_recoil_pass_name_)};
  auto recoil_track_collection{event.getCollection<ldmx::Track>(
      recoil_track_collection_name_, input_recoil_pass_name_)};

  bool passes_veto{false}, passes_tagger_veto{false}, passes_recoil_veto{false};

  // Start with tagger tracks
  int tagger_n{0};
  for (const auto& trk : tagger_track_collection) {
    if ((std::abs(trk.getD0()) < max_d0_) &&
        (std::abs(trk.getZ0()) < max_z0_)) {
      auto charge_over_momentum = trk.getQoP();
      auto tagger_momentum = 1000. / std::abs(charge_over_momentum);
      float chi2_per_ndf = trk.getChi2() / trk.getNdf();
      if ((tagger_momentum > min_tagger_momentum_) &&
          (chi2_per_ndf < max_chi2_per_ndf_) &&
          (trk.getNhits() > min_tagger_hits_)) {
        tagger_n++;
        passes_tagger_veto = true;
      }
    }
  }

  ldmx_log(info) << "Tagger requirements passed for " << tagger_n << " tracks";

  // Recoil tracks now
  int recoil_n{0};
  for (const auto& trk : recoil_track_collection) {
    float chi2_per_ndf = trk.getChi2() / trk.getNdf();
    if ((std::abs(trk.getD0()) < max_d0_) &&
        (std::abs(trk.getZ0()) < max_z0_) &&
        (chi2_per_ndf < max_chi2_per_ndf_) &&
        (trk.getNhits() > min_recoil_hits_)) {
      // we may wanna use these in the future, keeping them commented
      // auto charge_over_momentum = trk.getQoP();
      // recoil_momentum = 1000. / abs(charge_over_momentum);
      recoil_n++;
    };
  }

  ldmx_log(info) << "Recoil requirements passed for " << recoil_n << " tracks";

  if ((min_recoil_n_ <= recoil_n) && (recoil_n <= max_recoil_n_)) {
    passes_recoil_veto = true;
  }

  passes_veto = passes_tagger_veto && passes_recoil_veto;

  ldmx::TrackerVetoResult result;
  result.setVetoResult(passes_veto);
  result.setTaggerVetoResult(passes_tagger_veto);
  result.setRecoilVetoResult(passes_recoil_veto);

  event.add(output_collection_, result);

  if (!inverse_skim_) {
    if (passes_veto) {
      ldmx_log(info) << "Tracker veto passed, skim will keep the event";
      setStorageHint(framework::hint_shouldKeep);
    } else {
      ldmx_log(info) << "Tracker veto failed";
      setStorageHint(framework::hint_shouldDrop);
    }
  } else {
    if (passes_veto) {
      ldmx_log(info) << "Inverse tracker veto passed";
      setStorageHint(framework::hint_shouldDrop);
    } else {
      ldmx_log(info) << "Inverse tracker veto failed, skim will keep the event";
      setStorageHint(framework::hint_shouldKeep);
    }
  }
}  // produce
}  // namespace tracking

DECLARE_PRODUCER_NS(tracking, TrackerVetoProcessor);
