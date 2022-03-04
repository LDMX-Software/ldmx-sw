
#include "DQM/EcalDigiVerifier.h"

#include "SimCore/Event/SimCalorimeterHit.h"
#include "Ecal/Event/EcalHit.h"

namespace dqm {

void EcalDigiVerifier::configure(framework::config::Parameters &ps) {
  ecalSimHitColl_ = ps.getParameter<std::string>("ecalSimHitColl");
  ecalSimHitPass_ = ps.getParameter<std::string>("ecalSimHitPass");
  ecalRecHitColl_ = ps.getParameter<std::string>("ecalRecHitColl");
  ecalRecHitPass_ = ps.getParameter<std::string>("ecalRecHitPass");

  return;
}

void EcalDigiVerifier::analyze(const framework::Event &event) {
  // get truth information sorted into an ID based map
  std::vector<ldmx::SimCalorimeterHit> ecalSimHits =
      event.getCollection<ldmx::SimCalorimeterHit>(ecalSimHitColl_,
                                                   ecalSimHitPass_);

  // sort sim hits by ID
  std::sort(ecalSimHits.begin(), ecalSimHits.end(),
            [](const ldmx::SimCalorimeterHit &lhs,
               const ldmx::SimCalorimeterHit &rhs) {
              return lhs.getID() < rhs.getID();
            });

  std::vector<ldmx::EcalHit> ecalRecHits =
      event.getCollection<ldmx::EcalHit>(ecalRecHitColl_, ecalRecHitPass_);

  // sort rec hits by ID
  std::sort(ecalRecHits.begin(), ecalRecHits.end(),
            [](const ldmx::EcalHit &lhs, const ldmx::EcalHit &rhs) {
              return lhs.getID() < rhs.getID();
            });

  double totalRecEnergy = 0.;
  for (const ldmx::EcalHit &recHit : ecalRecHits) {
    // skip anything that digi flagged as noise
    if (recHit.isNoise()) continue;

    int rawID = recHit.getID();

    // get information for this hit
    int numSimHits = 0;
    double totalSimEDep = 0.;
    for (const ldmx::SimCalorimeterHit &simHit : ecalSimHits) {
      if (rawID == simHit.getID()) {
        numSimHits += simHit.getNumberOfContribs();
        totalSimEDep += simHit.getEdep();
      } else if (rawID < simHit.getID()) {
        // later sim hits - all done
        break;
      }
    }

    histograms_.fill("num_sim_hits_per_cell", numSimHits);
    histograms_.fill("sim_edep__rec_amplitude", totalSimEDep,
                     recHit.getAmplitude());

    totalRecEnergy += recHit.getEnergy();
  }

  histograms_.fill("total_rec_energy", totalRecEnergy);

  if (totalRecEnergy > 6000.) {
    setStorageHint(framework::hint_shouldKeep);
  } else {
    setStorageHint(framework::hint_shouldDrop);
  }

  return;
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, EcalDigiVerifier);
