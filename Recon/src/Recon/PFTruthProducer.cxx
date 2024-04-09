#include "Recon/PFTruthProducer.h"

#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"

namespace recon {

void PFTruthProducer::configure(framework::config::Parameters& ps) {
  primaryCollName_ = ps.getParameter<std::string>("outputPrimaryCollName"); 
  targetCollName_  = ps.getParameter<std::string>("outputTargetCollName"); 
  ecalCollName_	   = ps.getParameter<std::string>("outputEcalCollName"); 
  hcalCollName_	   = ps.getParameter<std::string>("outputHcalCollName"); 
}
template <class T>
void sortHits( std::vector<T> spHits){
  std::sort(spHits.begin(), spHits.end(),
	    [](T a, T b) {
	      return a.getEnergy() > b.getEnergy();
	    });
}

void PFTruthProducer::produce(framework::Event& event) {

  if (!event.exists("TargetScoringPlaneHits")) return;
  if (!event.exists("EcalScoringPlaneHits")) return;
  if (!event.exists("SimParticles")) return;
  const auto targSpHits = event.getCollection<ldmx::SimTrackerHit>("TargetScoringPlaneHits");
  const auto ecalSpHits = event.getCollection<ldmx::SimTrackerHit>("EcalScoringPlaneHits");
  const auto particle_map = event.getMap<int,ldmx::SimParticle>("SimParticles");

  std::map<int, ldmx::SimParticle> primaries;
  std::set<int> simIDs;
  std::vector<ldmx::SimTrackerHit> atTarget;
  std::vector<ldmx::SimTrackerHit> atEcal;
  std::vector<ldmx::SimTrackerHit> atHcal;
  for(const auto &pm : particle_map){
    const auto &p = pm.second;
    // the only parent of a primary is "track 0"
    if (p.getParents().size()==1 && p.getParents()[0]==0){
      primaries[pm.first] = p;
      simIDs.insert(pm.first);
    }
  }
  for(const auto &spHit : targSpHits){
    if ( simIDs.count(spHit.getTrackID()) && fabs(0.18-spHit.getPosition()[2])<0.1  && spHit.getMomentum()[2] > 0 ){ 
      atTarget.push_back(spHit);
    }
  }
  for(const auto &spHit : ecalSpHits){
    if ( simIDs.count(spHit.getTrackID()) && fabs(240-spHit.getPosition()[2])<0.1  && spHit.getMomentum()[2] > 0 ){ 
      atEcal.push_back(spHit);
    }
    if ( simIDs.count(spHit.getTrackID()) && fabs(840-spHit.getPosition()[2])<0.1  && spHit.getMomentum()[2] > 0 ){ 
      atHcal.push_back(spHit);
    }
  }
  // sortHits(primaries); // use map instead
  sortHits(atTarget);
  sortHits(atEcal);
  sortHits(atHcal);
  event.add(primaryCollName_, primaries);
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
