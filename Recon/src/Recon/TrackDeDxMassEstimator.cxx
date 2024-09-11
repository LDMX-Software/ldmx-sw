
#include "Recon/TrackDeDxMassEstimator.h"
#include "Tracking/Event/Track.h"

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
  auto tracks{event.getCollection<ldmx::Track>(trackCollection_)};
  
  
  

  // // Loop over the collection of hits and print the hit details
  // for (const ldmx::EcalHit &hit : hits) {
  //   // Print the hit
  //   hit.Print();
  // }
}
}  // namespace recon

DECLARE_PRODUCER_NS(recon, TrackDeDxMassEstimator)
