#include "Tracking/Reco/LinearTrackFinder.h"

//--- C++ StdLib ---//
#include <algorithm>
#include <iostream>
#include <map>
#include <typeinfo>

// eN files
#include <fstream>

namespace tracking {
namespace reco {

LinearTrackFinder::LinearTrackFinder(const std::string& name,
                                     framework::Process& process)
    : TrackingGeometryUser(name, process) {}

void LinearTrackFinder::onProcessStart() {}

void LinearTrackFinder::configure(framework::config::Parameters& parameters) {
  // seeds from the event
  seed_collection_ = parameters.getParameter<std::string>(
      "seed_collection", "LinearRecoilSeedTracks");
  // output track collection
  out_trk_collection_ = parameters.getParameter<std::string>(
      "out_trk_collection", "LinearRecoilTracks");
}

void LinearTrackFinder::produce(framework::Event& event) {
  std::vector<ldmx::StraightTrack> straight_tracks;

  auto start = std::chrono::high_resolution_clock::now();

  n_events_++;
  if (n_events_ % 1000 == 0) ldmx_log(info) << "events processed:" << n_events_;

  ldmx_log(debug) << "Retrieve the seeds::" << seed_collection_;

  const std::vector<ldmx::StraightTrack> seed_tracks =
      event.getCollection<ldmx::StraightTrack>(seed_collection_);

  n_seeds_ = seed_tracks.size();
  ldmx_log(debug) << "Number of seeds::" << n_seeds_;

  if (n_seeds_ > 0) {
    straight_tracks = findTracks(seed_tracks);
  }

  n_tracks_ += straight_tracks.size();

  // Add the tracks to the event
  event.add(out_trk_collection_, straight_tracks);

  auto end = std::chrono::high_resolution_clock::now();
  auto diff = end - start;
  processing_time_ += std::chrono::duration<double, std::milli>(diff).count();

  straight_tracks.clear();
}

void LinearTrackFinder::onProcessEnd() {
  ldmx_log(info) << "found " << n_tracks_ << " tracks / " << n_events_
                 << " events.";
  ldmx_log(info) << "AVG Time/Event: " << std::fixed << std::setprecision(1)
                 << processing_time_ / n_events_ << " ms";
}

std::vector<ldmx::StraightTrack> LinearTrackFinder::findTracks(
    const std::vector<ldmx::StraightTrack>& track_seeds) {
  std::vector<ldmx::StraightTrack> best_tracks;
  std::map<std::array<double, 3>, std::vector<ldmx::StraightTrack>>
      seeds_by_rec_hit;

  // Group seeds by their EcalRecHit point
  for (const auto& seed : track_seeds) {
    auto rec_hit_point = seed.getFirstLayerEcalRecHit();
    seeds_by_rec_hit[rec_hit_point].push_back(seed);
  }

  std::set<std::tuple<float, float, float>>
      used_sensor_positions;  // Track used sensor positions

  // Find the best seed for each RecHit
  // numTracks <= number of RecHits
  for (auto& entry : seeds_by_rec_hit) {
    const auto& rec_hit_point = entry.first;
    auto& seeds_with_same_rec_hit = entry.second;

    ldmx_log(debug) << "Processing RecHit at: (" << rec_hit_point[0] << ", "
                    << rec_hit_point[1] << ", " << rec_hit_point[2] << ")\n";

    // Main function to remove seeds with overlapping sensor positions
    seeds_with_same_rec_hit.erase(
        std::remove_if(
            seeds_with_same_rec_hit.begin(), seeds_with_same_rec_hit.end(),
            [&](const ldmx::StraightTrack& seed) {
              for (const auto& measurement : seed.getAllSensorPoints()) {
                // Check if this sensor's position is already used
                if (isPositionUsed(measurement, used_sensor_positions)) {
                  return true;  // Mark this seed for removal
                }               // if
              }                 // for
              return false;
            }),  // Keep the seed if no position overlap
        seeds_with_same_rec_hit.end());

    // If no valid seeds remain after filtering, skip to next RecHit
    if (seeds_with_same_rec_hit.empty()) continue;

    // Find the seed with the lowest chi2 for this RecHit, this is "best" seed
    auto best_seed_it = std::min_element(
        seeds_with_same_rec_hit.begin(), seeds_with_same_rec_hit.end(),
        [](const ldmx::StraightTrack& a, const ldmx::StraightTrack& b) {
          return a.getChi2() < b.getChi2();
        });

    // Store the best seed for this RecHit
    ldmx::StraightTrack best_seed = *best_seed_it;
    best_tracks.push_back(best_seed);

    ldmx_log(debug) << "For RecHit at: (" << rec_hit_point[0] << ", "
                    << rec_hit_point[1] << ", " << rec_hit_point[2] << ")\n";

    // Add best seed's sensor position to the global used positions set
    auto best_seed_measurement = best_seed.getAllSensorPoints();

    for (auto& position_object : best_seed_measurement) {
      used_sensor_positions.insert(
          std::make_tuple(position_object.getGlobalPosition()[0],
                          position_object.getGlobalPosition()[1],
                          position_object.getGlobalPosition()[2]));
      ldmx_log(debug) << "We used the following point: ("
                      << position_object.getGlobalPosition()[0] << ", "
                      << position_object.getGlobalPosition()[1] << ", "
                      << position_object.getGlobalPosition()[2] << ")\n";
      ldmx_log(debug) << "Which gave a track a distance: "
                      << best_seed.getDistanceToRecHit()
                      << " to the closest ECalRecHit\n";
    }  // for sensor points in the "best" seed

  }  // for entry loop, everytime we loop onto a new RecHit, we will have fewer
     // points to check

  return best_tracks;

}  // findTracks

bool LinearTrackFinder::isPositionUsed(
    const ldmx::Measurement& measurement,
    const std::set<std::tuple<float, float, float>>& used_sensor_positions) {
  const auto& position = std::make_tuple(measurement.getGlobalPosition()[0],
                                         measurement.getGlobalPosition()[1],
                                         measurement.getGlobalPosition()[2]);

  return used_sensor_positions.find(position) != used_sensor_positions.end();

}  // isPositionUsed

}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER_NS(tracking::reco, LinearTrackFinder)
