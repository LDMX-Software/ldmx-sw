#include "Tracking/Reco/GreedyAmbiguitySolver.h"

#include <algorithm>

#include "Acts/EventData/TrackHelpers.hpp"
#include "Acts/EventData/SourceLink.hpp"

namespace tracking {
namespace reco {

GreedyAmbiguitySolver::GreedyAmbiguitySolver(const std::string& name, framework::Process& process)
    : TrackingGeometryUser(name, process) {}

GreedyAmbiguitySolver::~GreedyAmbiguitySolver() {}

// Helper Functions

/*  
std::size_t GreedyAmbiguitySolver::sourceLinkHash(const Acts::SourceLink& a) { 
  return static_cast<std::size_t>(
      a.get<ActsExamples::IndexSourceLink>().index());
    }

bool GreedyAmbiguitySolver::sourceLinkEquality(const Acts::SourceLink& a, const Acts::SourceLink& b) {
  return a.get<ActsExamples::IndexSourceLink>().index() ==
         b.get<ActsExamples::IndexSourceLink>().index();
}
*/

void GreedyAmbiguitySolver::removeTrack(State& state,
                        std::size_t iTrack) const {
  for (auto iMeasurement : state.measurementsPerTrack[iTrack]) {
    state.tracksPerMeasurement[iMeasurement].erase(iTrack);
    if (state.tracksPerMeasurement[iMeasurement].size() == 1) {
      auto jTrack = *state.tracksPerMeasurement[iMeasurement].begin();
      --state.sharedMeasurementsPerTrack[jTrack];
    }
  }
  state.selectedTracks.erase(iTrack);
}

template <typename geometry_t, typename source_link_hash_t,
            typename source_link_equality_t>
void GreedyAmbiguitySolver::computeInitialState(
      std::vector<ldmx::Track> tracks,  std::vector<ldmx::Measurement> meas_coll,
      State& state, geometry_t& tg, source_link_hash_t&& sourceLinkHash,
      source_link_equality_t&& sourceLinkEquality) const {
  
  auto measurementIndexMap =
      std::unordered_map<Acts::SourceLink, std::size_t, source_link_hash_t,
                         source_link_equality_t>(0, sourceLinkHash,
                                                 sourceLinkEquality);

   //auto tg{geometry()};
  // Iterate through all input tracks, collect their properties like measurement
  // count and chi2 and fill the measurement map in order to relate tracks to
  // each other if they have shared hits.
  state.numberOfTracks = 0;
  for (const auto& track : tracks) {

    // Kick out tracks that do not fulfill our initial requirements
    if (track.getNhits() < nMeasurementsMin_) {
      continue;
    }

    std::vector<std::size_t> measurements;
    for (auto imeas : track.getMeasurementsIdxs()) {
        auto meas = meas_coll.at(imeas);
        const Acts::Surface* hit_surface = tg.getSurface(meas.getLayerID());
        // Store the index source link
        ActsExamples::IndexSourceLink idx_sl(hit_surface->geometryId(), imeas);
        Acts::SourceLink sourceLink = Acts::SourceLink(idx_sl);
   
        auto emplace = measurementIndexMap.try_emplace(
            sourceLink, measurementIndexMap.size());
        measurements.push_back(emplace.first->second);
    }

    state.trackTips.push_back(state.numberOfTracks);
    state.trackChi2.push_back(track.getChi2() / track.getNdf());
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

void GreedyAmbiguitySolver::resolve(State& state) const {
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

  for (std::size_t i = 0; i < maximumIterations_; ++i) {
    // Lazy out if there is nothing to filter on.
    if (state.selectedTracks.empty()) {
      //ldmx_log(debug) << "no tracks left - exit loop";
      break;
    }

    // Find the maximum amount of shared measurements per track to decide if we
    // are done or not.
    auto maximumSharedMeasurements = *std::max_element(
        state.selectedTracks.begin(), state.selectedTracks.end(),
        sharedMeasurementsComperator);
    //ldmx_log(debug)  <<
    //    "maximum shared measurements "
    //    << state.sharedMeasurementsPerTrack[maximumSharedMeasurements];
    if (state.sharedMeasurementsPerTrack[maximumSharedMeasurements] <
        maximumSharedHits_) {
      break;
    }

    // Find the "worst" track by comparing them to each other
    auto badTrack =
        *std::max_element(state.selectedTracks.begin(),
                          state.selectedTracks.end(), trackComperator);
    //ldmx_log(debug)  << "remove track "
    //            << badTrack << " nMeas "
    //             << state.measurementsPerTrack[badTrack].size() << " nShared "
    //             << state.sharedMeasurementsPerTrack[badTrack] << " chi2 "
    //             << state.trackChi2[badTrack] << std::endl;
    removeTrack(state, badTrack);
  }
}

// Processor Functions

void GreedyAmbiguitySolver::onNewRun(const ldmx::RunHeader& rh) {}

void GreedyAmbiguitySolver::configure(framework::config::Parameters& parameters) {
    out_trk_collection_ =
      parameters.getParameter<std::string>("out_trk_collection", "TaggerTracksClean");

    trackCollection_= parameters.getParameter<std::string>("trackCollection", "TaggerTracks");

    measCollection_= parameters.getParameter<std::string>("measCollection", "DigiTaggerSimHits");

    nMeasurementsMin_ = parameters.getParameter<int>("nMeasurementsMin",5);
    maximumSharedHits_ = parameters.getParameter<int>("maximumSharedHits",1);
}

void GreedyAmbiguitySolver::produce(framework::Event& event) {
    GreedyAmbiguitySolver::State state;
    std::vector<ldmx::Track> out_tracks;

    auto tg{geometry()};

    if (!event.exists(trackCollection_)) return;
    auto tracks{event.getCollection<ldmx::Track>(trackCollection_)};

    if (!event.exists(measCollection_)) return;
    auto measurements{event.getCollection<ldmx::Measurement>(measCollection_)};

    computeInitialState(tracks, measurements, state, tg, tracking::sim::utils::sourceLinkHash, tracking::sim::utils::sourceLinkEquality);
    resolve(state);

    for (auto iTrack : state.selectedTracks) {
      auto clean_trk = tracks[state.trackTips.at(iTrack)];
      if (clean_trk.getNhits() > nMeasurementsMin_ && abs(1. / clean_trk.getQoP()) > 0.05) {
        out_tracks.push_back(clean_trk);
     }
    }

    event.add(out_trk_collection_, out_tracks);


    //for (auto iTrack : initial_state.selectedTracks) {
    //    std::cout << event.getEventNumber() << " " << iTrack << " " << initial_state.trackChi2[iTrack] << " " << initial_state.measurementsPerTrack[iTrack].size() << std::endl;
    //}

    ldmx_log(debug) <<  " " << "Resolved to " << state.selectedTracks.size() << " tracks from "
                           << " " << tracks.size();

}

void GreedyAmbiguitySolver::onProcessStart(){};
void GreedyAmbiguitySolver::onProcessEnd(){};

}
}

DECLARE_PRODUCER_NS(tracking::reco, GreedyAmbiguitySolver)
