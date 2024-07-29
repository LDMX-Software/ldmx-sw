// This file is part of the Acts project.
//
// Copyright (C) 2020 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Framework/RandomNumberSeedService.h"

#include "Acts/EventData/MultiTrajectoryHelpers.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/Utilities/Delegate.hpp"
#include "Acts/Utilities/Logger.hpp"

#include <memory>
#include <unordered_map>
#include <iostream>

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>

namespace tracking::reco {

/// Evicts tracks that seem to be duplicates or fakes. This algorithm takes a
/// greedy approach in the sense that it will remove the track which looks "most
/// duplicate/fake" first and continues the same process with the rest. That
/// process continues until the final state conditions are met.
///
/// The implementation works as follows:
///  1) Calculate shared hits per track.
///  2) If the maximum shared hits criteria is met, we are done.
///     This is the configurable amount of shared hits we are ok with
///     in our experiment.
///  3) Else, remove the track with the highest relative shared hits (i.e.
///     shared hits / hits).
///  4) Back to square 1.
class GreedyAmbiguityResolution {
 public:
  struct Config {
    /// Maximum amount of shared hits per track.
    std::uint32_t maximumSharedHits = 1;
    /// Maximum number of iterations
    std::uint32_t maximumIterations = 1000;

    /// Minimum number of measurement to form a track.
    std::size_t nMeasurementsMin = 7;
  };

  struct State {
    std::size_t numberOfTracks{};

    std::vector<int> trackTips;
    std::vector<float> trackChi2;
    std::vector<std::vector<std::size_t>> measurementsPerTrack;

    // TODO consider boost 1.81 unordered_flat_map
    boost::container::flat_map<std::size_t,
                               boost::container::flat_set<std::size_t>>
        tracksPerMeasurement;
    std::vector<std::size_t> sharedMeasurementsPerTrack;

    // TODO consider boost 1.81 unordered_flat_map
    boost::container::flat_set<std::size_t> selectedTracks;
  };

  GreedyAmbiguityResolution(const Config& cfg,
                            std::unique_ptr<const Acts::Logger> logger =
                                Acts::getDefaultLogger("GreedyAmbiguityResolution",
                                                 Acts::Logging::INFO))
      : m_cfg{cfg}, m_logger{std::move(logger)} {}

  /// Computes the initial state for the input data. This function accumulates
  /// information that will later be used to accelerate the ambiguity
  /// resolution.
  ///
  /// @param tracks The input track container.
  /// @param state An empty state object which is expected to be default constructed.
  /// @param sourceLinkHash A functor to acquire a hash from a given source link.
  /// @param sourceLinkEquality A functor to check equality of two source links.
  template <typename track_container_t, typename traj_t,
            template <typename> class holder_t, typename source_link_hash_t,
            typename source_link_equality_t>
  void computeInitialState(
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
        //std::cout << " NOT ENOUGH MEASUREMENTS!!" << std::endl;
      continue;
    }
    std::vector<std::size_t> measurements;
    for (auto ts : track.trackStates()) {
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


  /// Updates the state iteratively by evicting one track after the other until
  /// the final state conditions are met.
  ///
  /// @param state A state object that was previously filled by the initialization.
  void resolve(State& state) const;

 private:
  Config m_cfg;

  /// Logging instance
  std::unique_ptr<const Acts::Logger> m_logger;

  /// Private access to logging instance
  const Acts::Logger& logger() const { return *m_logger; }
};

}  // namespace Acts

//#include "Acts/AmbiguityResolution/GreedyAmbiguityResolution.ipp"