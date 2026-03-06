#include "Ecal/EcalHelper.h"

namespace ecal {

std::vector<float> trackProp(const ldmx::Tracks &tracks,
                             ldmx::TrackStateType ts_type,
                             const std::string &ts_title) {
  // Vector to hold the new track state variables
  std::vector<float> new_track_states;

  // Return if no tracks
  if (tracks.empty()) return new_track_states;

  // Otherwise loop on the tracks
  for (auto &track : tracks) {
    // Get track state for ts_type
    auto trk_ts = track.getTrackState(ts_type);
    // Continue if there's no value
    if (!trk_ts.has_value()) continue;
    ldmx::Track::TrackState ecal_track_state = trk_ts.value();

    // Check that the track state is filled
    if (ecal_track_state.pos_.size() < 3 || ecal_track_state.mom_.size() < 3)
      continue;

    // pos_ is (x, y, z) in mm (LDMX global); mom_ is (px, py, pz) in MeV
    new_track_states.push_back(static_cast<float>(ecal_track_state.pos_[0]));
    new_track_states.push_back(static_cast<float>(ecal_track_state.pos_[1]));
    new_track_states.push_back(static_cast<float>(ecal_track_state.pos_[2]));
    new_track_states.push_back(static_cast<float>(ecal_track_state.mom_[0]));
    new_track_states.push_back(static_cast<float>(ecal_track_state.mom_[1]));
    new_track_states.push_back(static_cast<float>(ecal_track_state.mom_[2]));

    // Break after getting the first valid track state
    // TODO: interface this with CLUE to make sure the propagated track
    //       has an associated cluster in the ECAL
    break;
  }

  return new_track_states;
}

// MIP tracking functions:

float distTwoLines(ROOT::Math::XYZVector v1, ROOT::Math::XYZVector v2,
                   ROOT::Math::XYZVector w1, ROOT::Math::XYZVector w2) {
  ROOT::Math::XYZVector e1 = v1 - v2;
  ROOT::Math::XYZVector e2 = w1 - w2;
  ROOT::Math::XYZVector crs = e1.Cross(e2);
  if (crs.R() == 0) {
    return 100.0;  // arbitrary large number; edge case that shouldn't cause
                   // problems.
  } else {
    return std::abs(crs.Dot(v1 - w1) / crs.R());
  }
}

float distPtToLine(ROOT::Math::XYZVector h1, ROOT::Math::XYZVector p1,
                   ROOT::Math::XYZVector p2) {
  return ((h1 - p1).Cross(h1 - p2)).R() / (p1 - p2).R();
}

}  // namespace ecal
