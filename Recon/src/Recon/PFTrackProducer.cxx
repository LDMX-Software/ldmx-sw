#include "Recon/PFTrackProducer.h"

#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"

namespace recon {

void PFTrackProducer::configure(framework::config::Parameters& ps) {
  inputTrackCollName_ = ps.getParameter<std::string>("inputTrackCollName");
  input_pass_name_ = ps.getParameter<std::string>("input_pass_name");
  outputTrackCollName_ = ps.getParameter<std::string>("outputTrackCollName");
  doElectronTracking_ = ps.getParameter<bool>("doElectronTracking");
}

double getP(const ldmx::SimTrackerHit& tk) {
  std::vector<double> pxyz = tk.getMomentum();
  return sqrt(pow(pxyz[0], 2) + pow(pxyz[1], 2) + pow(pxyz[2], 2));
}

void PFTrackProducer::produce(framework::Event& event) {
  if (!event.exists(inputTrackCollName_, input_pass_name_)) {
    ldmx_log(fatal) << "Couldn't find input collection " << inputTrackCollName_
                    << "_" << input_pass_name_;
    return;
  }
  const auto ecalSpHits = event.getCollection<ldmx::SimTrackerHit>(
      inputTrackCollName_, input_pass_name_);

  std::vector<ldmx::SimTrackerHit> pfTracks;
  if (truthTracking_) {
    for (const auto& spHit : ecalSpHits) {
      if (spHit.getPdgID() == 22 || spHit.getPdgID() == 2112) continue;
      if (fabs(240 - spHit.getPosition()[2]) > 0.1) continue;
      if (doElectronTracking_) {  // only select electron SP hits
        if (spHit.getPdgID() != 11) continue;
        if (spHit.getTrackID() < 2 && spHit.getMomentum()[2] > 2500) {
          // this is almost guaranteed to be a pileup beam electron! keep it
          pfTracks.push_back(spHit);
          ldmx_log(debug) << "Added beam electron SP hit: trackID="
                          << spHit.getTrackID()
                          << ", pz = " << spHit.getMomentum()[2];
        } else if (spHit.getTrackID() <= 30 && spHit.getMomentum()[2] > 5) {
          // require more than minimum forward momentum to catch recoil electron
          // candidates
          pfTracks.push_back(spHit);
          ldmx_log(debug) << "Adding SP hit: trackID=" << spHit.getTrackID()
                          << ", pdgID= " << spHit.getPdgID()
                          << ", pz = " << spHit.getMomentum()[2];
          continue;
        }
      }  // if electron tracking
      else {
        if (spHit.getTrackID() != 1 ||
            fabs(240 - spHit.getPosition()[2]) > 0.1 ||
            spHit.getMomentum()[2] < 0)
          continue;
        pfTracks.push_back(spHit);
        ldmx_log(debug) << "Adding SP hit: trackID=" << spHit.getTrackID()
                        << ", pdgID= " << spHit.getPdgID()
                        << ", pz = " << spHit.getMomentum()[2];
        break;
      }
    }  // over SP hits
  }  // do truth tracking
  std::sort(pfTracks.begin(), pfTracks.end(),
            [](ldmx::SimTrackerHit a, ldmx::SimTrackerHit b) {
              return getP(a) > getP(b);
            });
  event.add(outputTrackCollName_, pfTracks);
}
}  // namespace recon

DECLARE_PRODUCER_NS(recon, PFTrackProducer);
