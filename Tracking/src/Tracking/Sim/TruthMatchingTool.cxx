#include "Tracking/Reco/TruthMatchingTool.h"

namespace tracking {
namespace sim {

TruthMatchingTool::TruthInfo TruthMatchingTool::evaluate(
    const std::unordered_map<unsigned int, unsigned int>& trk_trackIDs,
    int n_meas) {
  TruthInfo ti;
  ti.truth_prob_ = 0.;
  ti.trackID = -1;
  ti.pdgID = 0;

  for (std::unordered_map<unsigned int, unsigned int>::const_iterator it =
           trk_trackIDs.begin();
       it != trk_trackIDs.end(); it++) {
    double current_truth_prob = (double)it->second / (double)n_meas;
    if (current_truth_prob > ti.truth_prob_) {
      ti.truth_prob_ = current_truth_prob;
      ti.trackID = it->first;
    }
  }

  if (ti.trackID > 0) ti.pdgID = map_[ti.trackID].getPdgID();

  return ti;
}

TruthMatchingTool::TruthInfo TruthMatchingTool::truthMatch(
    const std::vector<ldmx::Measurement>& vmeas) {
  std::unordered_map<unsigned int, unsigned int> trk_track_i_ds;

  for (auto meas : vmeas) {
    for (auto trk_id : meas.getTrackIds()) {
      if (trk_track_i_ds.find(trk_id) != trk_track_i_ds.end())
        trk_track_i_ds[trk_id]++;
      else
        trk_track_i_ds[trk_id] = 1;
    }
  }  // loop on measurements

  return evaluate(trk_track_i_ds, vmeas.size());
}

/**
 * Performs the truth matching by checking all the trackIDs associated to the
 * measurements on track
 * @param trk The track that needs to be truth matched
 * @return TruthInfo a struct containing the truth information regarding the
 * track Each measurement weights the same.
 *
 */

TruthMatchingTool::TruthInfo TruthMatchingTool::truthMatch(
    const ldmx::Track& trk) {
  // Map holding all tracksIds and their frequency
  std::unordered_map<unsigned int, unsigned int> trk_track_i_ds;

  for (auto meas_id : trk.getMeasurementsIdxs()) {
    auto meas = measurements_.at(meas_id);
    if (debug_) {
      std::cout << "Getting measurement at ID:" << meas_id << std::endl;
      std::cout << meas << std::endl;
      std::cout << meas.getTrackIds().size() << std::endl;
    }

    for (auto trk_id : meas.getTrackIds()) {
      if (trk_track_i_ds.find(trk_id) != trk_track_i_ds.end()) {
        trk_track_i_ds[trk_id]++;
      } else {
        trk_track_i_ds[trk_id] = 1;
      }

    }  // loop on measurements trackIDs

    if (debug_) {
      std::cout << "The trackIDs maps look like:" << std::endl;
      for (std::unordered_map<unsigned int, unsigned int>::iterator it =
               trk_track_i_ds.begin();
           it != trk_track_i_ds.end(); it++) {
        std::cout << it->first << " " << it->second << std::endl;
      }
    }
  }  // loop on measurements

  return evaluate(trk_track_i_ds, trk.getMeasurementsIdxs().size());

}  // Match Track

}  // namespace sim
}  // namespace tracking
