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
    ldmx::Track::TrackState &ecal_track_state = trk_ts.value();

    // Check that the track state is filled
    if (ecal_track_state.params_.size() < 5) continue;

    float track_state_loc0 = static_cast<float>(ecal_track_state.params_[0]);
    float track_state_loc1 = static_cast<float>(ecal_track_state.params_[1]);
    // param 2 = phi (azimuthal), param 3 = theta (polar)
    // param 4 = QoP
    // ACTS (local)  to  LDMX (global) coordinates: (y_,z_,x_)->  (x_,y_,z_)
    // convert qop [1/GeV] to p [MeV]
    float p_track_state = (-1 / ecal_track_state.params_[4]) * 1000;
    // p * sin(theta) * sin(phi)
    float recoil_mom_x = p_track_state * sin(ecal_track_state.params_[3]) *
                         sin(ecal_track_state.params_[2]);
    // p * cos(theta)
    float recoil_mom_y = p_track_state * cos(ecal_track_state.params_[3]);
    // p * sin(theta) * cos(phi)
    float recoil_mom_z = p_track_state * sin(ecal_track_state.params_[3]) *
                         cos(ecal_track_state.params_[2]);

    // Store the new track state variables
    new_track_states.push_back(track_state_loc0);
    new_track_states.push_back(track_state_loc1);
    // z_-position at the ECAL (4) or Target (1)
    if (ts_type == 4) {
      // this should match `ECAL_SCORING_PLANE` in CKFProcessor
      new_track_states.push_back(240.5);
    } else if (ts_type == 1) {
      // This should match `target_surface` in CKFProcessor
      new_track_states.push_back(0.0);
    }

    new_track_states.push_back(recoil_mom_x);
    new_track_states.push_back(recoil_mom_y);
    new_track_states.push_back(recoil_mom_z);

    // Break after getting the first valid track state
    // TODO: interface this with CLUE to make sure the propageted track
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
