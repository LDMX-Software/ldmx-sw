#include "Ecal/EcalHelper.h"

namespace ecal {

std::vector<float> trackProp(const ldmx::Tracks& tracks,
                             ldmx::TrackStateType ts_type,
                             const std::string& ts_title) {
  // Vector to hold the new track state variables
  std::vector<float> new_track_states;

  // Return if no tracks
  if (tracks.empty()) return new_track_states;

  // Otherwise loop on the tracks
  for (auto& track : tracks) {
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

// Returns 'ele_count' tracks with the greatest transverse momentum that is also
// valid at the Ecal face
std::vector<std::vector<float>> pTTrackProp(const ldmx::Tracks& tracks,
                                            int ele_count) {
  // Vector to hold the new track state variables, indexed by pT
  std::vector<std::pair<float, std::vector<float>>> new_track_states;

  // Return empty vector if no tracks
  if (tracks.empty()) return {};

  // Otherwise loop on the tracks
  for (auto& track : tracks) {
    // Vector to hold track state parameters for a single track
    std::vector<float> track_state_vars;
    track_state_vars.reserve(6);
    // Get track state for Ecal
    auto trk_ts = track.getTrackState(ldmx::TrackStateType::AtECAL);
    // Continue if there's no value
    if (!trk_ts.has_value()) continue;
    ldmx::Track::TrackState ecal_track_state = trk_ts.value();

    // Check that the track state is filled
    if (ecal_track_state.pos_.size() < 3 || ecal_track_state.mom_.size() < 3)
      continue;

    // Calculate transverse momentum
    float transverse_momentum =
        sqrt((ecal_track_state.mom_[0] * ecal_track_state.mom_[0]) +
             (ecal_track_state.mom_[1] * ecal_track_state.mom_[1]));

    // store state variables
    track_state_vars.push_back(ecal_track_state.pos_[0]);
    track_state_vars.push_back(ecal_track_state.pos_[1]);
    track_state_vars.push_back(ecal_track_state.pos_[2]);
    track_state_vars.push_back(ecal_track_state.mom_[0]);
    track_state_vars.push_back(ecal_track_state.mom_[1]);
    track_state_vars.push_back(ecal_track_state.mom_[2]);

    // index track by total momentum into output
    new_track_states.emplace_back(transverse_momentum,
                                  std::move(track_state_vars));
  }

  // filters to get only the [ele_count] number of highest pT tracks
  std::sort(new_track_states.begin(), new_track_states.end(),
            [](const auto& a, const auto& b) {
              return a.first > b.first;
            });  // sort descending
  if (new_track_states.size() > ele_count) new_track_states.resize(ele_count);

  // Outputs the 'ele_count' track states themselves without the momentum
  // indexing
  std::vector<std::vector<float>> max_p_t_track_states;
  max_p_t_track_states.reserve(new_track_states.size());
  std::transform(std::make_move_iterator(new_track_states.begin()),
                 std::make_move_iterator(new_track_states.end()),
                 std::back_inserter(max_p_t_track_states),
                 [](auto&& ts) { return std::move(ts.second); });

  return max_p_t_track_states;
}

// Returns a specified number `ele_count` of highest momentum tracks which are
// valid at the Ecal face
std::vector<std::vector<float>> momTrackProp(const ldmx::Tracks& tracks,
                                             int ele_count) {
  // Vector variable to hold track state parameters, indexed by total momentum
  std::vector<std::pair<float, std::vector<float>>> new_track_states;

  // Return empty vector if no tracks
  if (tracks.empty()) return {};

  // Otherwise loop on the tracks
  for (auto& track : tracks) {
    // Vector to hold track state parameters for a single track
    std::vector<float> track_state_vars;
    track_state_vars.reserve(6);
    // Get track state for Ecal
    auto trk_ts = track.getTrackState(ldmx::TrackStateType::AtECAL);
    // Continue if there's no value
    if (!trk_ts.has_value()) continue;
    ldmx::Track::TrackState ecal_track_state = trk_ts.value();

    // Check that the track state is filled
    if (ecal_track_state.pos_.size() < 3 || ecal_track_state.mom_.size() < 3)
      continue;

    float total_momentum =
        std::sqrt(ecal_track_state.mom_[0] * ecal_track_state.mom_[0] +
                  ecal_track_state.mom_[1] * ecal_track_state.mom_[1] +
                  ecal_track_state.mom_[2] * ecal_track_state.mom_[2]);

    // store state variables
    track_state_vars.push_back(ecal_track_state.pos_[0]);
    track_state_vars.push_back(ecal_track_state.pos_[1]);
    track_state_vars.push_back(ecal_track_state.pos_[2]);
    track_state_vars.push_back(ecal_track_state.mom_[0]);
    track_state_vars.push_back(ecal_track_state.mom_[1]);
    track_state_vars.push_back(ecal_track_state.mom_[2]);

    // index track by total momentum into output
    new_track_states.emplace_back(total_momentum, std::move(track_state_vars));
  }

  // filters to get only the [ele_count] number of highest momentum tracks
  std::sort(new_track_states.begin(), new_track_states.end(),
            [](const auto& a, const auto& b) {
              return a.first > b.first;
            });  // sort descending
  if (new_track_states.size() > ele_count) new_track_states.resize(ele_count);

  // Outputs the [ele_count] track states themselves without the momentum
  // indexing
  std::vector<std::vector<float>> max_p_track_states;
  max_p_track_states.reserve(new_track_states.size());
  std::transform(std::make_move_iterator(new_track_states.begin()),
                 std::make_move_iterator(new_track_states.end()),
                 std::back_inserter(max_p_track_states),
                 [](auto&& ts) { return std::move(ts.second); });

  return max_p_track_states;
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
