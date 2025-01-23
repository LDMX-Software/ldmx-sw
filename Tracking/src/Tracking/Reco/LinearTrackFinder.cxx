#include "Tracking/Reco/LinearTrackFinder.h"

#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/Utilities/TrackHelpers.hpp"
#include "SimCore/Event/SimParticle.h"
#include "Tracking/Reco/TruthMatchingTool.h"
#include "Tracking/Sim/GeometryContainers.h"

//--- C++ StdLib ---//
#include <map>
#include <algorithm>  //std::vector reverse
#include <iostream>
#include <typeinfo>
// eN files
#include <fstream>

namespace tracking {
namespace reco {

LinearTrackFinder::LinearTrackFinder(const std::string& name, framework::Process& process)
: TrackingGeometryUser(name, process) {}

LinearTrackFinder::~LinearTrackFinder() {}

void LinearTrackFinder::onProcessStart() {}

void LinearTrackFinder::configure(framework::config::Parameters& parameters) {
    // seeds from the event
    seed_coll_name_ = parameters.getParameter<std::string>("seed_coll_name", "LinearRecoilSeedTracks");
    // output track collection
    out_trk_collection_ = parameters.getParameter<std::string>("out_trk_collection", "LinearRecoilTracks");
}

void LinearTrackFinder::produce(framework::Event& event) {
    std::vector<ldmx::StraightTrack> straight_tracks;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    nevents_++;
    if (nevents_ % 1000 == 0) ldmx_log(info) << "events processed:" << nevents_;
    
    ldmx_log(debug) << "Retrieve the seeds::" << seed_coll_name_;
    
    const std::vector<ldmx::StraightTrack> seed_tracks = event.getCollection<ldmx::StraightTrack>(seed_coll_name_);
    
    nseeds_ = seed_tracks.size();
    ldmx_log(debug) << "Number of seeds::" << nseeds_;
    
    if (nseeds_ > 0) {
        straight_tracks = findTracks(seed_tracks);
    }
    
    ntracks_ += straight_tracks.size();
    
    // Add the tracks to the event
    event.add(out_trk_collection_, straight_tracks);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto diff = end - start;
    processing_time_ += std::chrono::duration<double, std::milli>(diff).count();
    
    straight_tracks.clear();
}

void LinearTrackFinder::onProcessEnd() {
    ldmx_log(info) << "found " << ntracks_ << " tracks / " << nevents_ << " events.";
    ldmx_log(info) << "AVG Time/Event: " << std::fixed << std::setprecision(1) << processing_time_ / nevents_ << " ms";
}

std::vector<ldmx::StraightTrack> LinearTrackFinder::findTracks(const std::vector<ldmx::StraightTrack>& trackSeeds) {
    
    std::vector<ldmx::StraightTrack> bestTracks;
    std::map<std::array<double, 3>, std::vector<ldmx::StraightTrack>> seedsByRecHit;
    
    // Group seeds by their EcalRecHit point
    for (const auto& seed : trackSeeds) {
        auto recHitPoint = seed.getFirstLayerEcalRecHit();
        seedsByRecHit[recHitPoint].push_back(seed);
    }
    
    std::set<std::tuple<float, float, float>> usedSensorPositions; //Track used sensor positions
    
    // Process seedsByRecHit: iterate over the sorted keys and find the best seed for each RecHit
    for (auto& entry : seedsByRecHit) {
        const auto& recHitPoint = entry.first;
        auto& seedsWithSameRecHit = entry.second;
        
        ldmx_log(debug) << "Processing RecHit at: ("
        << recHitPoint[0] << ", "
        << recHitPoint[1] << ", "
        << recHitPoint[2] << ")\n";
        
        // Main function to remove seeds with overlapping sensor positions
        seedsWithSameRecHit.erase( std::remove_if(seedsWithSameRecHit.begin(), seedsWithSameRecHit.end(),
                                                  [&](const ldmx::StraightTrack& seed) {
                                                        for (const auto& measurement : seed.getAllSensorPoints()) {
                                                        // Check if this sensor point's position is already used
                                                            if (isPositionUsed(measurement, usedSensorPositions)) {
                                                                return true; // Mark seed for removal
                                                            } //if
                                                        } //for
                                                        return false;}), // Keep the seed if no position overlaps
                                    seedsWithSameRecHit.end() );
        
        // If no valid seeds remain after filtering, skip to next RecHit
        if (seedsWithSameRecHit.empty()) continue;
        
        // Find the seed with the lowest chi2 for this RecHit
        auto bestSeedIt = std::min_element(seedsWithSameRecHit.begin(),
                                           seedsWithSameRecHit.end(),
                                           [](const ldmx::StraightTrack& a, const ldmx::StraightTrack& b) {
            return a.getChi2() < b.getChi2();
        });
        
        // Store the best seed for this RecHit
        ldmx::StraightTrack bestSeed = *bestSeedIt;
        bestTracks.push_back(bestSeed);
        
        ldmx_log(debug) << "For RecHit at: ("
        << recHitPoint[0] << ", "
        << recHitPoint[1] << ", "
        << recHitPoint[2] << ")\n";
        
        // Add best seed's sensor position to the global used positions set
        auto bestSeedMeasurement = bestSeed.getAllSensorPoints();
        for (auto& positionObject : bestSeedMeasurement) {
            usedSensorPositions.insert(std::make_tuple(positionObject.getGlobalPosition()[0], positionObject.getGlobalPosition()[1], positionObject.getGlobalPosition()[2]));
            ldmx_log(debug) << "We used the following point: ("
            << positionObject.getGlobalPosition()[0] << ", "
            << positionObject.getGlobalPosition()[1] << ", "
            << positionObject.getGlobalPosition()[2] << ")\n";
            ldmx_log(debug) << "Which gave a track a distance: " << bestSeed.getDistanceToRecHit() << " to the closest ECalRecHit\n";
        }
    } //for entry loop
    return bestTracks;
}

// Helper function to check if a measurement's position is already used
bool LinearTrackFinder::isPositionUsed(const ldmx::Measurement& measurement,
                    const std::set<std::tuple<float, float, float>>& usedSensorPositions) {
    const auto& position = std::make_tuple(measurement.getGlobalPosition()[0], measurement.getGlobalPosition()[1], measurement.getGlobalPosition()[2]);
    return usedSensorPositions.find(position) != usedSensorPositions.end();
} //isPositionUsed
    
}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER_NS(tracking::reco, LinearTrackFinder)
