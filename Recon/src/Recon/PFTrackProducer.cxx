#include "Recon/PFTrackProducer.h"

#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"

namespace recon {

void PFTrackProducer::configure(framework::config::Parameters& ps) {
  inputTrackCollName_ = ps.getParameter<std::string>("inputTrackCollName");
  outputTrackCollName_ = ps.getParameter<std::string>("outputTrackCollName");
}

double getP(const ldmx::SimTrackerHit& tk) {
  std::vector<double> pxyz = tk.getMomentum();
  return sqrt(pow(pxyz[0], 2) + pow(pxyz[1], 2) + pow(pxyz[2], 2));
}

void PFTrackProducer::produce(framework::Event& event) {
  if (!event.exists(inputTrackCollName_)) {
    ldmx_log(fatal) << "Input track collection not found";
    return;
  }
  const auto ecalSpHits =
      event.getCollection<ldmx::SimTrackerHit>(inputTrackCollName_);

  std::vector<ldmx::SimTrackerHit> pfTracks;
  if (truthTracking_) {
    for (const auto& spHit : ecalSpHits) {
      if (spHit.getTrackID() != 1 || fabs(240 - spHit.getPosition()[2]) > 0.1 ||
          spHit.getMomentum()[2] <= 0)
        continue;
      if (spHit.getPdgID() == 22 || spHit.getPdgID() == 2112) continue;
      pfTracks.push_back(spHit);
      break;
    }
  }
  std::sort(pfTracks.begin(), pfTracks.end(),
            [](ldmx::SimTrackerHit a, ldmx::SimTrackerHit b) {
              return getP(a) > getP(b);
            });
  event.add(outputTrackCollName_, pfTracks);
}

void PFTrackProducer::onFileOpen() {
  ldmx_log(debug) << "Opening file!";

  return;
}

void PFTrackProducer::onFileClose() {
  ldmx_log(debug) << "Closing file!";

  return;
}

void PFTrackProducer::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  return;
}

void PFTrackProducer::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}

}  // namespace recon

DECLARE_PRODUCER_NS(recon, PFTrackProducer);
