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
  trackCollection_ =
      ps.getParameter<std::string>("track_collection", "RecoilTracks");

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
  std::transform(trackColl.begin(), trackColl.end(), trackColl.begin(), ::tolower);
  if (trackColl.find("tagger") != std::string::npos) {
    trackType = 1;
  } else if (trackColl.find("recoil") != std::string::npos) {
    trackType = 2;
  } else {
    trackType = -1;
  }

  // Retrieve the measurements
  if (!event.exists(measCollection_)) return;
  auto measurements{event.getCollection<ldmx::Measurement>(measCollection_)};

  std::vector<ldmx::TrackDeDxMassEstimate> massEstimates;

  // Loop over the collection of tracks
  // for (auto &track : tracks) {
  for (uint i = 0; i < tracks.size(); i++) {
    auto track = tracks.at(i);
    // If track momentum doen't exist, skip
    auto trackMomentum = track.getMomentum();
    if (trackMomentum.size() == 0) {
      ldmx_log(debug) << "Track " << i << " is empty";
      // std::cout << "Track " << i << " is empty" << std::endl;
      continue;
    }

    auto px = trackMomentum[0] * 1000;  // GeV to MeV
    auto py = trackMomentum[1] * 1000;
    auto pz = trackMomentum[2] * 1000;
    // Get the track momentum magnitude
    float p = sqrt(px * px + py * py + pz * pz);
    ldmx_log(debug) << "Track " << i << " has momentum " << p;
    // std::cout << "Track " << i << " has momentum " << p << std::endl;

    float pathLength = si_sensor_thickness_mm * p / abs(pz);  // For now, use global angle
    // std::cout << "Track " << i << " has p " << p << " MeV and pz " << pz << "MeV" << std::endl;
    // std::cout << "Track " << i << " has path length " << pathLength << "mm" << std::endl;
    if (pathLength <= 0) {
      ldmx_log(debug) << "Track " << i << " has path length " << pathLength << "mm";
      continue;
    }

    /// Get the hits associated with the track
    ldmx::TrackDeDxMassEstimate massEst;
    float sum_dEdx_inv2 = 0.;
    float dEdx;
    
    int n_mes = 0;
    for (auto imeas : track.getMeasurementsIdxs()) {
      auto meas = measurements.at(imeas);
      if (meas.getEdep() >= 0) {
        dEdx = meas.getEdep() / pathLength * 10;  // unit: MeV/cm
        // std::cout << "  Measurement " << n_mes << " dEdx: " << dEdx 
                  << ", edep " << meas.getEdep() << std::endl;
        sum_dEdx_inv2 += 1. / (dEdx * dEdx);
        n_mes++;
      }
    }  // end of loop over measurements

    if (sum_dEdx_inv2 == 0) {
      ldmx_log(debug) << "Track " << i << " has no dEdx measurements";
      // std::cout << "Track " << i << " has no dEdx measurements" << std::endl;
      continue;
    }
    // Ih = (1/N * sum_i^N(dE/dx_i)^-2)^-1/2
    float Ih = 1. / sqrt(1. / n_mes * sum_dEdx_inv2);

    float mass = 0.;
    if (Ih > fit_res_C_) {
      mass = p * sqrt((Ih - fit_res_C_) / fit_res_K_);
    }
    else {
      ldmx_log(debug) << "Track " << i << " has Ih " << Ih << " which is less than fit_res_C " << fit_res_C_;
      // std::cout << "Track " << i << " has Ih " << Ih << " which is less than fit_res_C " << fit_res_C_ << std::endl;
    }

    massEst.setMass(mass);
    massEst.setTrackIndex(i);
    massEst.setTrackType(trackType);
    massEstimates.push_back(massEst);
  }

  // Add the mass estimates to the event
  event.add("TrackDeDxMassEstimate", massEstimates);

  // // Loop over the collection of hits and print the hit details
  // for (const ldmx::EcalHit &hit : hits) {
  //   // Print the hit
  //   hit.Print();
  // }
}
}  // namespace recon

DECLARE_PRODUCER_NS(recon, TrackDeDxMassEstimator)
