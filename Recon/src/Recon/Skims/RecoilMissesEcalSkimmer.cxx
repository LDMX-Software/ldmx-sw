/**
 * @file RecoilMissesEcalSkimmer.cxx
 * @brief Processor used to select events where the recoil electron misses the
 *        Ecal.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Recon/Skims/RecoilMissesEcalSkimmer.h"

namespace ldmx {

RecoilMissesEcalSkimmer::RecoilMissesEcalSkimmer(const std::string &name,
                                                 Process &process)
    : Producer(name, process) {}

RecoilMissesEcalSkimmer::~RecoilMissesEcalSkimmer() {}

void RecoilMissesEcalSkimmer::produce(Event &event) {
  // Get the collection of simulated particles from the event
  auto particleMap{event.getMap<int, SimParticle>("SimParticles")};

  // Search for the recoil electron
  auto [recoilTrackID, recoilElectron] = Analysis::getRecoil(particleMap);

  // Get the collection of simulated Ecal hits from the event.
  const std::vector<SimCalorimeterHit> ecalSimHits =
      event.getCollection<SimCalorimeterHit>("EcalSimHits");

  // Loop through the Ecal hits and check if the recoil electron is
  // associated with any of them.  If there are any recoil electron hits
  // in the Ecal, drop the event.
  bool hasRecoilElectronHits = false;
  for (const SimCalorimeterHit &simHit : ecalSimHits) {
    /*std::cout << "[ RecoilMissesEcalSkimmer ]: "
              << "Number of hit contributions: "
              << simHit->getNumberOfContribs() << std::endl;*/

    for (int iContrib = 0; iContrib < simHit.getNumberOfContribs();
         ++iContrib) {
      SimCalorimeterHit::Contrib contrib = simHit.getContrib(iContrib);

      if (contrib.trackID == recoilTrackID) {
        /*std::cout << "[ RecoilMissesEcalSkimmer ]: "
                  << "Ecal hit associated with recoil electron." << std::endl;
         */

        hasRecoilElectronHits = true;
      }
    }
  }

  // Tell the skimmer to keep or drop the event based on whether there
  // were recoil electron hits found in the Ecal.
  if (hasRecoilElectronHits) {
    setStorageHint(hint_shouldDrop);
  } else {
    setStorageHint(hint_shouldKeep);
  }
}
}  // namespace ldmx

DECLARE_PRODUCER_NS(ldmx, RecoilMissesEcalSkimmer);
