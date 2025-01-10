// LDMX
#include "Recon/TrackDeDxMassEstimator.h"

#include "Recon/Event/TrackDeDxMassEstimate.h"

// STL
#include <algorithm>  // for std::transform
#include <cctype>     // for ::tolower
#include <iostream>

namespace recon {

void TrackDeDxMassEstimator::configure(framework::config::Parameters &ps) {
  fit_res_C_ = ps.getParameter<double>("fit_res_C");
  fit_res_K_ = ps.getParameter<double>("fit_res_K");
  trackCollection_ =
      ps.getParameter<std::string>("track_collection", "RecoilTruthTracks");

  ldmx_log(info) << "Track Collection used for TrackDeDxMassEstimator "
                 << trackCollection_;
}

void TrackDeDxMassEstimator::produce(framework::Event &event) {
  if (!event.exists(trackCollection_)) {
    ldmx_log(error) << "ERROR:: trackCollection " << trackCollection_
                    << " not in event" << std::endl;
    return;
  }
  const std::vector<ldmx::Track> tracks{
      event.getCollection<ldmx::Track>(trackCollection_)};

  int trackType;
  std::string trackColl = trackCollection_;
  std::transform(trackColl.begin(), trackColl.end(), trackColl.begin(),
                 ::tolower);
  if (trackColl.find("tagger") != std::string::npos) {
    trackType = 1;
    simhitCollection_ = "TaggerSimHits";
  } else if (trackColl.find("recoil") != std::string::npos) {
    trackType = 2;
    simhitCollection_ = "RecoilSimHits";
  } else {
    trackType = 0;
    simhitCollection_ = "";
  }

  // Retrieve the simhits
  if (!event.exists(simhitCollection_)) return;
  auto simhits{event.getCollection<ldmx::SimTrackerHit>(simhitCollection_)};

  std::vector<ldmx::TrackDeDxMassEstimate> massEstimates;

  // Loop over the collection of tracks
  for (uint i = 0; i < tracks.size(); i++) {
    auto track = tracks.at(i);
    // If track momentum doen't exist, skip
    auto QoP = track.getQoP();
    if (QoP == 0) {
      ldmx_log(debug) << "Track " << i << "has zero q/p ";
      continue;
    }

    float p = 1. / abs(QoP) * 1000;  // unit: MeV
    ldmx_log(debug) << "Track " << i << " has momentum " << p;

    /// Get the hits associated with the truth track
    ldmx::TrackDeDxMassEstimate massEst;
    float sum_dEdx_inv2 = 0.;
    float dEdx;
    float n_simhits = 0;
    for (auto hit : simhits) {
      // Check if the hit is associated with the track
      // std::cout << "hit trackID: " << hit.getTrackID() << ", trackID: " <<
      // track.getTrackID() << std::endl;
      if (hit.getTrackID() != track.getTrackID()) continue;
      if (hit.getEdep() >= 0 && hit.getPathLength() > 0) {
        dEdx = hit.getEdep() / hit.getPathLength() * 10;  // unit: MeV/cm
        // std::cout << "  SimHit " << n_simhits << " dEdx: " << dEdx
        //           << ", edep " << hit.getEdep() << ", path length " <<
        //           hit.getPathLength()
        //           << std::endl;
        sum_dEdx_inv2 += 1. / (dEdx * dEdx);
        n_simhits++;
      }
    }  // end of loop over measurements

    if (sum_dEdx_inv2 == 0) {
      ldmx_log(debug) << "Track " << i << " has no dEdx measurements";
      continue;
    }

    // Ih = (1/N * sum_i^N(dE/dx_i)^-2)^-1/2
    float Ih = 1. / sqrt(1. / n_simhits * sum_dEdx_inv2);

    float mass = 0.;
    if (Ih > fit_res_C_) {
      mass = p * sqrt((Ih - fit_res_C_) / fit_res_K_);
    } else {
      ldmx_log(info) << "Track " << i << " has Ih " << Ih
                     << " which is less than fit_res_C " << fit_res_C_;
      mass = -100.;
    }

    massEst.setMass(mass);
    massEst.setTrackIndex(i);
    massEst.setTrackType(trackType);
    massEstimates.push_back(massEst);
  }

  // Add the mass estimates to the event
  event.add("TrackDeDxMassEstimate", massEstimates);
}
}  // namespace recon

DECLARE_PRODUCER_NS(recon, TrackDeDxMassEstimator)
