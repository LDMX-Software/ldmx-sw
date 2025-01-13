// LDMX
#include "Recon/TrackDeDxMassEstimator.h"
#include "Recon/Event/TrackDeDxMassEstimate.h"

// STL
#include <iostream>
#include <algorithm> // for std::transform
#include <cctype>    // for ::tolower

namespace recon {

void TrackDeDxMassEstimator::configure(framework::config::Parameters &ps) {
  fit_res_C_ = ps.getParameter<double>("fit_res_C");
  fit_res_K_ = ps.getParameter<double>("fit_res_K");
  track_collection_ =
      ps.getParameter<std::string>("track_collection", "RecoilTruthTracks");

  ldmx_log(info) << "Track Collection used for TrackDeDxMassEstimator "
                 << track_collection_;
}

void TrackDeDxMassEstimator::produce(framework::Event &event) {
  
  if (!event.exists(track_collection_)) {
    ldmx_log(error) << "ERROR:: track collection " << track_collection_
                    << " not in event" << std::endl;
    return;
  }
  const std::vector<ldmx::Track> tracks{
      event.getCollection<ldmx::Track>(track_collection_)};
  
  int track_type;
  std::string track_coll_str = track_collection_;
  std::transform(track_coll_str.begin(), track_coll_str.end(), track_coll_str.begin(), ::tolower);
  if (track_coll_str.find("tagger") != std::string::npos) {
    track_type = 1;
    simhit_collection_ = "TaggerSimHits";
  } else if (track_coll_str.find("recoil") != std::string::npos) {
    track_type = 2;
    simhit_collection_ = "RecoilSimHits";
  } else {
    track_type = 0;
    simhit_collection_ = "";
  }

  // Retrieve the simhits
  if (!event.exists(simhit_collection_)) return;
  auto simhits{event.getCollection<ldmx::SimTrackerHit>(simhit_collection_)};

  std::vector<ldmx::TrackDeDxMassEstimate> mass_estimates_;

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
    ldmx::TrackDeDxMassEstimate mass_est;
    float sum_dEdx_inv2 = 0.;
    float dEdx;
    float n_simhits = 0;
    for (auto hit : simhits) {
      // Check if the hit is associated with the track
      if (hit.getTrackID() != track.getTrackID()) continue;
      if (hit.getEdep() >= 0 && hit.getPathLength() > 0) {
        dEdx = hit.getEdep() / hit.getPathLength() * 10;  // unit: MeV/cm
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
    }
    else {
      ldmx_log(info) << "Track " << i << " has Ih " << Ih << " which is less than fit_res_C " << fit_res_C_;
      mass = -100.;
    }

    mass_est.setMass(mass);
    mass_est.setTrackIndex(i);
    mass_est.setTrackType(track_type);
    mass_estimates_.push_back(mass_est);
  }

  // Add the mass estimates to the event
  event.add("TrackDeDxMassEstimate", mass_estimates_);

}
}  // namespace recon

DECLARE_PRODUCER_NS(recon, TrackDeDxMassEstimator)
