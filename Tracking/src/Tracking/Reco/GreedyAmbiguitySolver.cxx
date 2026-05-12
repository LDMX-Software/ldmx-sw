#include "Tracking/Reco/GreedyAmbiguitySolver.h"

#include <algorithm>

#include "Acts/EventData/SourceLink.hpp"
#include "Acts/EventData/TrackHelpers.hpp"

namespace tracking {
namespace reco {

GreedyAmbiguitySolver::GreedyAmbiguitySolver(const std::string& name,
                                             framework::Process& process)
    : TrackingGeometryUser(name, process) {}

// Helper Functions

/*
std::size_t GreedyAmbiguitySolver::sourceLinkHash(const Acts::SourceLink& a) {
  return static_cast<std::size_t>(
      a.get<ActsExamples::IndexSourceLink>().index());
    }

bool GreedyAmbiguitySolver::sourceLinkEquality(const Acts::SourceLink& a, const
Acts::SourceLink& b) { return a.get<ActsExamples::IndexSourceLink>().index() ==
         b.get<ActsExamples::IndexSourceLink>().index();
}
*/

void GreedyAmbiguitySolver::removeTrack(State& state,
                                        std::size_t iTrack) const {
  for (auto i_measurement : state.measurements_per_track_[iTrack]) {
    state.tracks_per_measurement_[i_measurement].erase(iTrack);
    if (state.tracks_per_measurement_[i_measurement].size() == 1) {
      auto j_track = *state.tracks_per_measurement_[i_measurement].begin();
      --state.shared_measurements_per_track_[j_track];
    }
  }
  state.selected_tracks_.erase(iTrack);
}

template <typename geometry_t, typename source_link_hash_t,
          typename source_link_equality_t>
void GreedyAmbiguitySolver::computeInitialState(
    std::vector<ldmx::Track> tracks, std::vector<ldmx::Measurement> meas_coll,
    State& state, geometry_t& tg, source_link_hash_t&& sourceLinkHash,
    source_link_equality_t&& sourceLinkEquality) const {
  auto measurement_index_map =
      std::unordered_map<Acts::SourceLink, std::size_t, source_link_hash_t,
                         source_link_equality_t>(0, sourceLinkHash,
                                                 sourceLinkEquality);

  // auto tg{geometry()};
  // Iterate through all input tracks, collect their properties like measurement
  // count and chi2 and fill the measurement map in order to relate tracks to
  // each other if they have shared hits_.
  state.number_of_tracks_ = 0;
  for (const auto& track : tracks) {
    // Kick out tracks that do not fulfill our initial requirements
    if (track.getNhits() < n_meas_min_) {
      continue;
    }

    std::vector<std::size_t> measurements;
    for (auto imeas : track.getMeasurementsIdxs()) {
      auto meas = meas_coll.at(imeas);
      const Acts::Surface* hit_surface = tg.getSurface(meas.getLayerID());
      // Store the index_ source link
      acts_examples::IndexSourceLink idx_sl(hit_surface->geometryId(), imeas);
      Acts::SourceLink source_link = Acts::SourceLink(idx_sl);

      auto emplace = measurement_index_map.try_emplace(
          source_link, measurement_index_map.size());
      measurements.push_back(emplace.first->second);
    }

    state.track_tips_.push_back(state.number_of_tracks_);
    state.track_chi2_.push_back(track.getChi2() / track.getNdf());
    state.measurements_per_track_.push_back(std::move(measurements));
    state.selected_tracks_.insert(state.number_of_tracks_);

    ++state.number_of_tracks_;
  }

  // Now we relate measurements to tracks
  for (std::size_t i_track = 0; i_track < state.number_of_tracks_; ++i_track) {
    for (auto i_measurement : state.measurements_per_track_[i_track]) {
      state.tracks_per_measurement_[i_measurement].insert(i_track);
    }
  }

  // Finally, we can accumulate the number of shared measurements per track
  state.shared_measurements_per_track_ =
      std::vector<std::size_t>(state.track_tips_.size(), 0);
  for (std::size_t i_track = 0; i_track < state.number_of_tracks_; ++i_track) {
    for (auto i_measurement : state.measurements_per_track_[i_track]) {
      if (state.tracks_per_measurement_[i_measurement].size() > 1) {
        ++state.shared_measurements_per_track_[i_track];
      }
    }
  }
}

void GreedyAmbiguitySolver::resolve(State& state) {
  /// Compares two tracks based on the number of shared measurements in order to
  /// decide if we already met the final state.
  auto shared_measurements_comperator = [&state](std::size_t a, std::size_t b) {
    return state.shared_measurements_per_track_[a] <
           state.shared_measurements_per_track_[b];
  };

  /// Compares two tracks in order to find the one which should be evicted.
  /// First we compare the relative amount of shared measurements. If that is
  /// indecisive we use the chi2.
  auto track_comperator = [&state](std::size_t a, std::size_t b) {
    /// Helper to calculate the relative amount of shared measurements.
    auto relative_shared_measurements = [&state](std::size_t i) {
      return 1.0 * state.shared_measurements_per_track_[i] /
             state.measurements_per_track_[i].size();
    };

    if (relative_shared_measurements(a) != relative_shared_measurements(b)) {
      return relative_shared_measurements(a) < relative_shared_measurements(b);
    }
    return state.track_chi2_[a] < state.track_chi2_[b];
  };

  for (std::size_t i = 0; i < maximum_iterations_; ++i) {
    // Lazy out if there is nothing to filter on.
    if (state.selected_tracks_.empty()) {
      ldmx_log(trace) << "No tracks left - exit loop";
      break;
    }

    // Find the maximum amount of shared measurements per track to decide if we
    // are done or not.
    auto maximum_shared_measurements = *std::max_element(
        state.selected_tracks_.begin(), state.selected_tracks_.end(),
        shared_measurements_comperator);
    // ldmx_log(debug)  <<
    //     "maximum shared measurements "
    //     << state.sharedMeasurementsPerTrack[maximumSharedMeasurements];
    if (state.shared_measurements_per_track_[maximum_shared_measurements] <
        maximum_shared_hits_) {
      break;
    }

    // Find the "worst" track by comparing them to each other
    auto bad_track =
        *std::max_element(state.selected_tracks_.begin(),
                          state.selected_tracks_.end(), track_comperator);
    ldmx_log(trace) << "Remove track " << bad_track << ", nMeas = "
                    << state.measurements_per_track_[bad_track].size()
                    << ", nShared = "
                    << state.shared_measurements_per_track_[bad_track]
                    << ", chi2 =" << state.track_chi2_[bad_track];
    removeTrack(state, bad_track);
  }
}

// Processor Functions

void GreedyAmbiguitySolver::onNewRun(const ldmx::RunHeader& rh) {}

void GreedyAmbiguitySolver::configure(
    framework::config::Parameters& parameters) {
  out_trk_collection_ =
      parameters.get<std::string>("out_trk_collection", "TaggerTracksClean");

  track_collection_ =
      parameters.get<std::string>("track_collection", "TaggerTracks");

  meas_collection_ =
      parameters.get<std::string>("meas_collection", "DigiTaggerSimHits");
  input_pass_name_ = parameters.get<std::string>("input_pass_name");
  n_meas_min_ = parameters.get<int>("n_measurements_min", 5);
  maximum_shared_hits_ = parameters.get<int>("maximum_shared_hits", 1);
}

void GreedyAmbiguitySolver::produce(framework::Event& event) {
  GreedyAmbiguitySolver::State state;
  std::vector<ldmx::Track> out_tracks;

  auto tg{geometry()};

  if (!event.exists(track_collection_, input_pass_name_)) {
    ldmx_log(debug) << "Track collection not found, exiting";
    return;
  }
  const auto& tracks =
      event.getCollection<ldmx::Track>(track_collection_, input_pass_name_);

  if (!event.exists(meas_collection_, input_pass_name_)) {
    ldmx_log(debug) << "Measurement collection not found, exiting";
    return;
  }
  const auto& measurements =
      event.getCollection<ldmx::Measurement>(meas_collection_, input_pass_name_);

  computeInitialState(tracks, measurements, state, tg,
                      tracking::sim::utils::sourceLinkHash,
                      tracking::sim::utils::sourceLinkEquality);
  resolve(state);

  for (auto i_track : state.selected_tracks_) {
    auto clean_trk = tracks[state.track_tips_.at(i_track)];
    if ((clean_trk.getNhits() > n_meas_min_) &&
        (std::abs(1. / clean_trk.getQoP()) > 0.05)) {
      out_tracks.push_back(clean_trk);
    }
  }

  event.add(out_trk_collection_, out_tracks);

  // for (auto iTrack : initial_state.selectedTracks) {
  //     std::cout << event.getEventNumber() << " " << iTrack << " " <<
  //     initial_state.trackChi2[iTrack] << " " <<
  //     initial_state.measurementsPerTrack[iTrack].size() << std::endl;
  // }

  ldmx_log(info) << " Resolved to " << state.selected_tracks_.size()
                 << " tracks from " << " " << tracks.size();
}

}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER(tracking::reco::GreedyAmbiguitySolver)
