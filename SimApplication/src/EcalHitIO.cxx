#include "SimApplication/EcalHitIO.h"

// STL
#include <map>

// LDMX
#include "Event/SimCalorimeterHit.h"
#include "Event/SimParticle.h"

namespace ldmx {

EcalHitIO::EcalHitIO(SimParticleBuilder* simParticleBuilder)
    : simParticleBuilder_(simParticleBuilder) {
}

void EcalHitIO::writeHitsCollection(G4CalorimeterHitsCollection* hc, TClonesArray* outputColl) {

    int nHits = hc->GetSize();
    std::map<int, SimCalorimeterHit*> hitMap;

    // Loop over input hits from Geant4.
    for (int iHit = 0; iHit < nHits; iHit++) {

        // Get the hit and its ID.
        G4CalorimeterHit* g4hit = (G4CalorimeterHit*) hc->GetHit(iHit);
        int hitID = g4hit->getID();

        // See if hit exists in map already.
        std::map<int, SimCalorimeterHit*>::iterator it = hitMap.find(hitID);
        SimCalorimeterHit* simHit;

        // Is it a new hit?
        if (it == hitMap.end()) {

            // Create sim hit and assign the ID.
            simHit = (SimCalorimeterHit*) outputColl->ConstructedAt(outputColl->GetEntries());
            simHit->setID(hitID);

            /**
             * Assign XY position to the hit using the ECal hex readout.
             * Z position is set from the original hit, which should be the middle of the sensor.
             */
            detID_.setRawValue(hitID);
            detID_.unpack();
            int cellID = detID_.getFieldValue("cell");
            std::pair<float,float> XYPair = hexReadout_.getCellCentroidXYPair(cellID);
            simHit->setPosition(XYPair.first, XYPair.second, g4hit->getPosition().z());

            hitMap[hitID] = simHit;

        } else {
            // Get existing hit from map.
            simHit = hitMap[hitID];
        }

        // Get info from the G4 hit.
        float edep = g4hit->getEdep();
        float time = g4hit->getTime();
        int pdgCode = g4hit->getPdgCode();

        // Is hit contrib output enabled?
        if (enableHitContribs_) {

            // Find the SimParticle associated with this hit.
            SimParticle* simParticle = simParticleBuilder_->findSimParticle(g4hit->getTrackID());

            // Find if there is an existing hit contrib.
            int contribIndex = simHit->findContribIndex(simParticle, pdgCode);

            // Is contrib output being compressed and a record exists for this SimParticle and PDG code?
            if (compressHitContribs_ && contribIndex != -1) {

                // Update an existing hit contrib.
                simHit->updateContrib(contribIndex, edep, time);

                //std::cout << "updated contrib for hit with ID " << hitID << " with PDGID = "
                //        << pdgCode << ", edep = " << edep << ", time = " << time << std::endl;;

            } else {

                // Add a hit contrib because all steps are being saved or there is not an existing record.
                simHit->addContrib(simParticle, pdgCode, edep, time);

                //std::cout << "added new contrib for hit with ID " << hitID << " with PDGID = "
                //        << pdgCode << ", edep = " << edep << ", time = " << time << std::endl;
            }
        } else {

            // Hit contributions are not being saved so manually increment the edep and set time.
            simHit->setEdep(simHit->getEdep() + edep);
            if (time < simHit->getTime() || simHit->getTime() == 0) {
                simHit->setTime(time);
            }

            //std::cout << "updated hit with ID " << hitID << " with edep = " << edep << ", time = " << time << std::endl;
        }
    }
}

} // namespace sim
