#include "Recon/TrackDeDxMassEstimator.h"

#include <iostream>

#include "Recon/Event/TrackDeDxMassEstimate.h"

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
      continue;
    }

    auto px = trackMomentum[0];
    auto py = trackMomentum[1];
    auto pz = trackMomentum[2];
    // Get the track momentum magnitude
    float p = sqrt(px * px + py * py + pz * pz);
    ldmx_log(debug) << "Track " << i << " has momentum " << p;

    /// Get the hits associated with the track
    ldmx::TrackDeDxMassEstimate mes;
    float sum_dedx = 0.;
    for (auto imeas : track.getMeasurementsIdxs()) {
      auto meas = measurements.at(imeas);
      if (meas.getEdep() > 0) sum_dedx += meas.getEdep();
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
