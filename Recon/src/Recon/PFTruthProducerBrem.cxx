#include "Recon/PFTruthProducerBrem.h"

#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "Recon/Event/PFCandidateBrem.h"


namespace recon {

void PFTruthProducerBrem::configure(framework::config::Parameters &ps) {
  primaryCollName_ = ps.getParameter<std::string>("outputPrimaryCollName");
  targetCollName_ = ps.getParameter<std::string>("outputTargetCollName");
  ecalCollName_ = ps.getParameter<std::string>("outputEcalCollName");
  hcalCollName_ = ps.getParameter<std::string>("outputHcalCollName");

  outputCollName_ = ps.getParameter<std::string>("outputCollName");
}
template <class T>
void sortHits(std::vector<T> spHits) {
  std::sort(spHits.begin(), spHits.end(),
            [](T a, T b) { return a.getEnergy() > b.getEnergy(); });
}

void PFTruthProducerBrem::produce(framework::Event &event) {
  if (!event.exists("TargetScoringPlaneHits")) return;
  if (!event.exists("EcalScoringPlaneHits")) return;
  if (!event.exists("SimParticles")) return;
  if (!event.exists("EcalSimHits")) return;

  const auto targSpHits =
      event.getCollection<ldmx::SimTrackerHit>("TargetScoringPlaneHits");
  const auto ecalSpHits =
      event.getCollection<ldmx::SimTrackerHit>("EcalScoringPlaneHits");
  const auto particle_map =
      event.getMap<int, ldmx::SimParticle>("SimParticles");
  const auto ecalSimHits =
      event.getCollection<ldmx::SimCalorimeterHit>("EcalSimHits");

  std::map<int, ldmx::SimParticle> primaries;
  std::set<int> simIDs;
  std::vector<ldmx::SimTrackerHit> atTarget;
  std::vector<ldmx::SimTrackerHit> atEcal;
  std::vector<ldmx::SimTrackerHit> atHcal;

  std::map<int, ldmx::PFCandidateBrem> pfCandsTruth;

  for (const auto &pm : particle_map) {
    const auto &p = pm.second;
    // the only parent of a primary is "track 0"
    if (p.getParents().size() == 1 && p.getParents()[0] == 0) {
      ldmx::PFCandidateBrem pfcand;
      primaries[pm.first] = p;
      simIDs.insert(pm.first);

      // Setting up the PFCandidate Info
      pfcand.setPID(pm.first);
      pfcand.setPDGId(p.getPdgID());
      pfcand.setMass(p.getMass());
      pfcand.setEnergy(p.getEnergy());
      
      // Now let's fill brem products
      int num_brems = 0;
      for (const auto &daughter: p.getDaughters()) {
        //std::cout << daughter << std::endl;
         // Finding the daughter in the particle_map
         auto it = particle_map.find(daughter);
         if (it != particle_map.end()) {
            // Checking if the daughter is a photon produced through a brem
            auto brem = particle_map.at(daughter);
            if ((brem.getPdgID() == 22) && brem.getProcessType() == 5) {
              pfcand.addBremProduct(daughter);
              pfcand.setBremProductEnergy(num_brems, brem.getEnergy());
              pfcand.setBremProductVertex(num_brems, brem.getVertexVolume());
              num_brems += 1;
            }
         }
      }
      pfCandsTruth[pm.first] = pfcand;
    }
  }

  // Now we have the ID's of all the primaries.

  for (const auto &spHit : targSpHits) {
    if (simIDs.count(spHit.getTrackID()) &&
        fabs(0.18 - spHit.getPosition()[2]) < 0.1 &&
        spHit.getMomentum()[2] > 0) { 
      int id = spHit.getTrackID();
      atTarget.push_back(spHit);
      pfCandsTruth.at(id).setTargetRecPositionXYZ(spHit.getPosition()[0], spHit.getPosition()[1], spHit.getPosition()[2]);
      pfCandsTruth.at(id).setRecoilTrackPxPyPz(spHit.getMomentum()[0], spHit.getMomentum()[1], spHit.getMomentum()[2]);
    }
    if (simIDs.count(spHit.getTrackID()) &&
        fabs(-0.18 - spHit.getPosition()[2]) < 0.1 &&
        spHit.getMomentum()[2] > 0) { 
      int id = spHit.getTrackID();
      pfCandsTruth.at(id).setTargetTagPositionXYZ(spHit.getPosition()[0], spHit.getPosition()[1], spHit.getPosition()[2]);
    }
  }
  for (const auto &spHit : ecalSpHits) {
    if (simIDs.count(spHit.getTrackID()) &&
        fabs(240 - spHit.getPosition()[2]) < 0.1 &&
        spHit.getMomentum()[2] > 0) {
      int id = spHit.getTrackID();
      atEcal.push_back(spHit);
      pfCandsTruth.at(id).setEcalPositionXYZ(spHit.getPosition()[0], spHit.getPosition()[1], spHit.getPosition()[2]);
      pfCandsTruth.at(id).setEcalMomentumXYZ(spHit.getMomentum()[0], spHit.getMomentum()[1], spHit.getMomentum()[2]);
      pfCandsTruth.at(id).setEcalEnergy(spHit.getEnergy());
      }

    for (auto &cand: pfCandsTruth) {
    auto brems = cand.second.getBremProductIDs();
      auto it = std::find(brems.begin(), brems.end(),spHit.getTrackID());
      if (it != brems.end()) {
        //int id = spHit.getTrackID();
        int index = it - brems.begin();
        cand.second.setBremProductECalPositionXYZ(index, spHit.getPosition()[0], spHit.getPosition()[1], spHit.getPosition()[2]);
      }
    }
    if (simIDs.count(spHit.getTrackID()) &&
        fabs(840 - spHit.getPosition()[2]) < 0.1 &&
        spHit.getMomentum()[2] > 0) {
      atHcal.push_back(spHit);
    }
  }



  /*
  Plan: iterate over SimHits and contrib IDs from the SimHit. 
  Get the incidentID corresponding to the maximum energy. If that
  is equal to one of the SimHit IDs, add. Or, if equal to one of the 
  bremIDs from that SimHit, add. Should be doable! But will start tomorrow.

  Then rest of the plan for tomorrow: Check that this information is accurate,
  and start writing actual PF matcher. Goal is to just do as much as possible.

  Then on weekend: psets and ML stuff. Then I should be all up!
  */

  for (const auto &eSimHit : ecalSimHits) {
    ldmx::SimCalorimeterHit::Contrib maxContrib;

    int maxContribEnergy = 0;
    for (int i = 0; i < eSimHit.getNumberOfContribs(); i++) {
      auto c = eSimHit.getContrib(i);
      if (c.edep > maxContribEnergy) {
        maxContribEnergy = c.edep;
        maxContrib = c;
      }
    }
    if (simIDs.count(maxContrib.incidentID))  {
      int id = maxContrib.incidentID;
      pfCandsTruth.at(id).addECalClusterHit(eSimHit.getID());
    }

    for (auto &cand: pfCandsTruth) {
    auto brems = cand.second.getBremProductIDs();
      auto it = std::find(brems.begin(), brems.end(),maxContrib.incidentID);
      if (it != brems.end()) {
        //int id = spHit.getTrackID();
        int index = it - brems.begin();
        cand.second.addBremProductECalHit(index, eSimHit.getID());
      }
    }
  }






  // sortHits(primaries); // use map instead
  //sortHits(atTarget);
  //sortHits(atEcal);
  //sortHits(atHcal);
  //event.add(primaryCollName_, primaries);
  //event.add(targetCollName_, atTarget);
  //event.add(ecalCollName_, atEcal);
  //event.add(hcalCollName_, atHcal);

  std::vector<ldmx::PFCandidateBrem> values;
    for (const auto& pair : pfCandsTruth) { 
        values.push_back(pair.second);
    }
  event.add(outputCollName_, values);
}
}  // namespace recon

DECLARE_PRODUCER_NS(recon, PFTruthProducerBrem);
