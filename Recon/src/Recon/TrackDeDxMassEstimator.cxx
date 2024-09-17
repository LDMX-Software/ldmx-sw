#include <iostream>
#include "Recon/TrackDeDxMassEstimator.h"
#include "Recon/Event/TrackDeDxMassEstimate.h"

namespace recon {

void TrackDeDxMassEstimator::configure(framework::config::Parameters &ps) {
  fit_res_C_ = ps.getParameter<double>("fit_res_C");
  fit_res_K_ = ps.getParameter<double>("fit_res_K");
  trackCollection_ =
      ps.getParameter<std::string>("track_collection", "RecoilTracks");
  
  ldmx_log(info) << "Track Collection " << trackCollection_ << std::endl;
}

void TrackDeDxMassEstimator::produce(framework::Event &event) {
  if (!event.exists(trackCollection_)) {
    ldmx_log(error) << "ERROR:: trackCollection " << trackCollection_
                    << " not in event" << std::endl;
    return;
  }
  const std::vector<ldmx::Track> tracks{event.getCollection<ldmx::Track>(trackCollection_)};

  // Retrieve the measurements
  if (!event.exists(measCollection_)) return;
  auto measurements{event.getCollection<ldmx::Measurement>(measCollection_)};

  std::vector<ldmx::TrackDeDxMassEstimate> massEstimates;
  
  // Loop over the collection of tracks
  // for (auto &track : tracks) {
  for (uint i = 0; i < tracks.size(); i++) {
    auto track = tracks.at(i);
    // If track momentum doen't exist, skip
    if (track.getMomentum().empty()) {
      ldmx_log(debug) << "Track " << i << " is empty" << std::endl;
      continue;
    }

    // Get the track momentum magnitude
    float p = sqrt(pow(track.getMomentum()[0], 2) 
                  + pow(track.getMomentum()[1], 2) 
                  + pow(track.getMomentum()[2], 2));
    std::cout << "Track " << i << " has momentum " << p << std::endl;

    /// Get the hits associated with the track
    ldmx::TrackDeDxMassEstimate mes;
    float sum_dedx = 0;
    for (auto imeas : track.getMeasurementsIdxs()) {
      auto meas = measurements.at(imeas);
      if (meas.getEdep() >= 0)
        sum_dedx += meas.getEdep();

    }
    mes.setMass(100.);
    mes.setTrackIndex(i);
    mes.setTrackType(2);
    massEstimates.push_back(mes);
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
