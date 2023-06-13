#include "Recon/PFTruthProducer.h"

#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"

namespace recon {

void PFTruthProducer::configure(framework::config::Parameters& ps) {
  targetCollName_ = ps.getParameter<std::string>("outputTargetCollName"); 
  ecalCollName_ = ps.getParameter<std::string>("outputEcalCollName"); 
  hcalCollName_ = ps.getParameter<std::string>("outputHcalCollName"); 
}

void sortHits( std::vector<ldmx::SimTrackerHit> spHits){
  std::sort(spHits.begin(), spHits.end(),
	    [](ldmx::SimTrackerHit a, ldmx::SimTrackerHit b) {
	      return a.getEnergy() > b.getEnergy();
	    });
}

void PFTruthProducer::produce(framework::Event& event) {

  if (!event.exists("TargetScoringPlaneHits")) return;
  if (!event.exists("EcalScoringPlaneHits")) return;
  const auto targSpHits = event.getCollection<ldmx::SimTrackerHit>("TargetScoringPlaneHits");
  const auto ecalSpHits = event.getCollection<ldmx::SimTrackerHit>("EcalScoringPlaneHits");

  std::vector<ldmx::SimTrackerHit> atTarget;
  std::vector<ldmx::SimTrackerHit> atEcal;
  std::vector<ldmx::SimTrackerHit> atHcal;
  for(const auto &spHit : targSpHits){
    if ( spHit.getTrackID()==1 && fabs(0.18-spHit.getPosition()[2])<0.1  && spHit.getMomentum()[2] > 0 ){ 
      atTarget.push_back(spHit);
    }
  }
  for(const auto &spHit : ecalSpHits){
    if ( spHit.getTrackID()==1 && fabs(240-spHit.getPosition()[2])<0.1  && spHit.getMomentum()[2] > 0 ){ 
      atEcal.push_back(spHit);
    }
    if ( spHit.getTrackID()==1 && fabs(840-spHit.getPosition()[2])<0.1  && spHit.getMomentum()[2] > 0 ){ 
      atHcal.push_back(spHit);
    }
  }
  sortHits(atTarget);
  sortHits(atEcal);
  sortHits(atHcal);
  event.add(targetCollName_, atTarget);
  event.add(ecalCollName_, atEcal);
  event.add(hcalCollName_, atHcal);
}

void PFTruthProducer::onFileOpen() {
  ldmx_log(debug) << "Opening file!";

  return;
}

void PFTruthProducer::onFileClose() {
  ldmx_log(debug) << "Closing file!";

  return;
}

void PFTruthProducer::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  return;
}

void PFTruthProducer::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}

}  // namespace recon

DECLARE_PRODUCER_NS(recon, PFTruthProducer);
