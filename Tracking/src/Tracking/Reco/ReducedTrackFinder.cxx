#include "Tracking/Reco/ReducedTrackFinder.h"

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

ReducedTrackFinder::ReducedTrackFinder(const std::string& name, framework::Process& process)
: TrackingGeometryUser(name, process) {}

ReducedTrackFinder::~ReducedTrackFinder() {}

void ReducedTrackFinder::onProcessStart() {}

void ReducedTrackFinder::configure(framework::config::Parameters& parameters) {
    // seeds from the event
    seed_coll_name_ = parameters.getParameter<std::string>("seed_coll_name", "ReducedSeedTracks");
    // output track collection
    out_trk_collection_ = parameters.getParameter<std::string>("out_trk_collection", "ReducedTracks");
}

void ReducedTrackFinder::produce(framework::Event& event) {
    std::vector<ldmx::ReducedTrack> reduced_tracks;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    nevents_++;
    if (nevents_ % 1000 == 0) ldmx_log(info) << "events processed:" << nevents_;
    
    ldmx_log(debug) << "Retrieve the seeds::" << seed_coll_name_;
    
    const std::vector<ldmx::ReducedTrack> seed_tracks = event.getCollection<ldmx::ReducedTrack>(seed_coll_name_);
    
    ldmx_log(debug) << "Number of seeds::" << seed_tracks.size();
    
    reduced_tracks = findTracks(seed_tracks);
    ntracks_ = reduced_tracks.size();
    
    // Add the tracks to the event
    event.add(out_trk_collection_, reduced_tracks);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto diff = end - start;
    processing_time_ += std::chrono::duration<double, std::milli>(diff).count();
}

void ReducedTrackFinder::onProcessEnd() {
    ldmx_log(info) << "found " << ntracks_ << " tracks  / " << nseeds_ << " nseeds";
    ldmx_log(info) << "AVG Time/Event: " << std::fixed << std::setprecision(1) << processing_time_ / nevents_ << " ms";
}

std::vector<ldmx::ReducedTrack> ReducedTrackFinder::findTracks(const std::vector<ldmx::ReducedTrack>& trackSeeds) {
    
    std::vector<ldmx::ReducedTrack> bestTrack;
    std::map<std::array<double, 3>, std::vector<ldmx::ReducedTrack>> seedsByRecHit;

    // Group seeds by their EcalRecHit point
    for (const auto& seed : trackSeeds) {
        auto recHitPoint = seed.getFirstLayerEcalRecHit();
        seedsByRecHit[recHitPoint].push_back(seed);
        
//        for (size_t i = 0; i < recHitPoint.size(); ++i) {
//            std::cout << "Element " << i << ": " << recHitPoint[i] << std::endl;
//        }
    }

    // Process seedsByRecHit: iterate over the sorted keys and find the best seed for each RecHit
    for (auto& entry : seedsByRecHit) {
        const auto& recHitPoint = entry.first;
        auto& seedsWithSameRecHit = entry.second;
        
        ldmx_log(debug) << "Processing RecHit at: ("
                        << recHitPoint[0] << ", "
                        << recHitPoint[1] << ", "
                        << recHitPoint[2] << ")\n";

        // Find the seed with the lowest chi2 for this RecHit
        auto bestSeedIt = std::min_element(seedsWithSameRecHit.begin(),
                                           seedsWithSameRecHit.end(),
                                           [](const ldmx::ReducedTrack& a, const ldmx::ReducedTrack& b) {
                                               return a.getChi2() < b.getChi2();
                                           });

        // Store the best seed for this RecHit
        ldmx::ReducedTrack bestSeed = *bestSeedIt;
        bestTrack.push_back(bestSeed);

        // Remove any seeds that share Recoil sensor points with the best seed
        auto sensorPoint1 = bestSeed.getFirstSensorPosition();
        auto sensorPoint2 = bestSeed.getSecondSensorPosition();
        seedsWithSameRecHit.erase(
            std::remove_if(seedsWithSameRecHit.begin(), seedsWithSameRecHit.end(),
                           [&](const ldmx::ReducedTrack& seed) {
                               auto recoilPoint1 = seed.getFirstSensorPosition();
                               auto recoilPoint2 = seed.getSecondSensorPosition();
                               return (sensorPoint1 == recoilPoint1 || sensorPoint1 == recoilPoint2 ||
                                       sensorPoint2 == recoilPoint1 || sensorPoint2 == recoilPoint2);
                           }),
            seedsWithSameRecHit.end());
    }

    return bestTrack;
}


}  // namespace reco
}  // namespace tracking

DECLARE_PRODUCER_NS(tracking::reco, ReducedTrackFinder)
