#include "SimApplication/EcalHitIO.h"

// STL
#include <map>

// LDMX
#include "Event/SimCalorimeterHit.h"
#include "Event/SimParticle.h"

using event::SimParticle;

namespace sim {

void EcalHitIO::writeHitsCollection(G4CalorimeterHitsCollection* hc, TClonesArray* outputColl, SimParticleBuilder* simParticleBuilder) {

    int nHits = hc->GetSize();

    std::map<int, event::SimCalorimeterHit*> hitMap;

    // Loop over input hits from Geant4.
    for (int iHit = 0; iHit < nHits; iHit++) {

        // Get the hit and its ID.
        G4CalorimeterHit* g4hit = (G4CalorimeterHit*) hc->GetHit(iHit);
        int hitID = g4hit->getID();

        // See if hit exists in map already.
        std::map<int, event::SimCalorimeterHit*>::iterator it = hitMap.find(hitID);
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
            detID.setRawValue(hitID);
            detID.unpack();
            int cellID = detID.getFieldValue("cell");
            std::pair<float,float> XYPair = hexReadout->getCellCentroidXYPair(cellID);
            simHit->setPosition(XYPair.first, XYPair.second, g4hit->getPosition().z());

            hitMap[hitID] = simHit;

            //std::cout << "created new hit with id " << hitID << std::endl;

        } else {
            simHit = hitMap[hitID];

            //std::cout << "found existing hit with id " << hitID << std::endl;
        }

        // Add hit contribution.
        float edep = g4hit->getEdep();
        float time = g4hit->getTime();
        SimParticle* simParticle = simParticleBuilder->findSimParticle(g4hit->getTrackID());
        int pdgCode = g4hit->getPdgCode();
        simHit->addContrib(simParticle, pdgCode, edep, time);
    }
}

} // namespace sim
