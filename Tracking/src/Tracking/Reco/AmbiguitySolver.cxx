// This file is part of the Acts project.
//
// Copyright (C) 2023 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Tracking/Reco/AmbiguitySolver.h"


namespace tracking::reco {

/* template <typename track_container_t, typename traj_t,
            template <typename> class holder_t, typename source_link_hash_t,
            typename source_link_equality_t>
void GreedyAmbiguityResolution::computeInitialState(
    const Acts::TrackContainer<track_container_t, traj_t, holder_t>& tracks,
    State& state, source_link_hash_t&& sourceLinkHash,
    source_link_equality_t&& sourceLinkEquality) const {
  auto measurementIndexMap =
      std::unordered_map<Acts::SourceLink, std::size_t, source_link_hash_t,
                         source_link_equality_t>(0, sourceLinkHash,
                                                 sourceLinkEquality);

  // Iterate through all input tracks, collect their properties like measurement
  // count and chi2 and fill the measurement map in order to relate tracks to
  // each other if they have shared hits.
  for (const auto& track : tracks) {
    // Kick out tracks that do not fulfill our initial requirements
    if (track.nMeasurements() < m_cfg.nMeasurementsMin) {
      continue;
    }
    std::vector<std::size_t> measurements;
    for (auto ts : track.trackStatesReversed()) {
      if (ts.typeFlags().test(Acts::TrackStateFlag::MeasurementFlag)) {
        Acts::SourceLink sourceLink = ts.getUncalibratedSourceLink();
        // assign a new measurement index if the source link was not seen yet
        auto emplace = measurementIndexMap.try_emplace(
            sourceLink, measurementIndexMap.size());
        measurements.push_back(emplace.first->second);
      }
    }

    state.trackTips.push_back(track.index());
    state.trackChi2.push_back(track.chi2() / track.nDoF());
    state.measurementsPerTrack.push_back(std::move(measurements));
    state.selectedTracks.insert(state.numberOfTracks);

    ++state.numberOfTracks;
  }

  // Now we relate measurements to tracks
  for (std::size_t iTrack = 0; iTrack < state.numberOfTracks; ++iTrack) {
    for (auto iMeasurement : state.measurementsPerTrack[iTrack]) {
      state.tracksPerMeasurement[iMeasurement].insert(iTrack);
    }
  }

  // Finally, we can accumulate the number of shared measurements per track
  state.sharedMeasurementsPerTrack =
      std::vector<std::size_t>(state.trackTips.size(), 0);
  for (std::size_t iTrack = 0; iTrack < state.numberOfTracks; ++iTrack) {
    for (auto iMeasurement : state.measurementsPerTrack[iTrack]) {
      if (state.tracksPerMeasurement[iMeasurement].size() > 1) {
        ++state.sharedMeasurementsPerTrack[iTrack];
      }
    }
  }
}
 */

//namespace {

/// Removes a track from the state which has to be done for multiple properties
/// because of redundancy.
static void removeTrack(GreedyAmbiguityResolution::State& state,
                        std::size_t iTrack) {
  for (auto iMeasurement : state.measurementsPerTrack[iTrack]) {
    state.tracksPerMeasurement[iMeasurement].erase(iTrack);

    if (state.tracksPerMeasurement[iMeasurement].size() == 1) {
      auto jTrack = *state.tracksPerMeasurement[iMeasurement].begin();
      --state.sharedMeasurementsPerTrack[jTrack];
    }
  }

  state.selectedTracks.erase(iTrack);
}

//}  // namespace

void GreedyAmbiguityResolution::resolve(State& state) const {
  /// Compares two tracks based on the number of shared measurements in order to
  /// decide if we already met the final state.
  auto sharedMeasurementsComperator = [&state](std::size_t a, std::size_t b) {
    return state.sharedMeasurementsPerTrack[a] <
           state.sharedMeasurementsPerTrack[b];
  };

  /// Compares two tracks in order to find the one which should be evicted.
  /// First we compare the relative amount of shared measurements. If that is
  /// indecisive we use the chi2.
  auto trackComperator = [&state](std::size_t a, std::size_t b) {
    /// Helper to calculate the relative amount of shared measurements.
    auto relativeSharedMeasurements = [&state](std::size_t i) {
      return 1.0 * state.sharedMeasurementsPerTrack[i] /
             state.measurementsPerTrack[i].size();
    };

    if (relativeSharedMeasurements(a) != relativeSharedMeasurements(b)) {
      return relativeSharedMeasurements(a) < relativeSharedMeasurements(b);
    }
    return state.trackChi2[a] < state.trackChi2[b];
  };

  for (std::size_t i = 0; i < m_cfg.maximumIterations; ++i) {
    // Lazy out if there is nothing to filter on.
    if (state.selectedTracks.empty()) {
      //std::cout << "no tracks left - exit loop" << std::endl;
      break;
    }

    // Find the maximum amount of shared measurements per track to decide if we
    // are done or not.
    auto maximumSharedMeasurements = *std::max_element(
        state.selectedTracks.begin(), state.selectedTracks.end(),
        sharedMeasurementsComperator);
    //std::cout <<
    //    "maximum shared measurements "
    //    << state.sharedMeasurementsPerTrack[maximumSharedMeasurements]  << std::endl;
    if (state.sharedMeasurementsPerTrack[maximumSharedMeasurements] <
        m_cfg.maximumSharedHits) {
      break;
    }

    // Find the "worst" track by comparing them to each other
    auto badTrack =
        *std::max_element(state.selectedTracks.begin(),
                          state.selectedTracks.end(), trackComperator);
    //std::cout << "remove track "
    //             << badTrack << " nMeas "
    //             << state.measurementsPerTrack[badTrack].size() << " nShared "
    //             << state.sharedMeasurementsPerTrack[badTrack] << " chi2 "
    //             << state.trackChi2[badTrack] << std::endl;
    removeTrack(state, badTrack);
  }
}

} 

