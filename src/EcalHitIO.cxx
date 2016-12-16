#include "SimApplication/EcalHitIO.h"

// STL
#include <map>

// LDMX
#include "Event/SimCalorimeterHit.h"
#include "Event/SimParticle.h"

using event::SimParticle;

namespace sim {

EcalHitIO::EcalHitIO(SimParticleBuilder* simParticleBuilder)
    : simParticleBuilder_(simParticleBuilder) {
}

void EcalHitIO::writeHitsCollection(G4CalorimeterHitsCollection* hc, TClonesArray* outputColl) {

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

        // Add hit contribution.
        float edep = g4hit->getEdep();
        float time = g4hit->getTime();
        SimParticle* simParticle = simParticleBuilder_->findSimParticle(g4hit->getTrackID());
        int pdgCode = g4hit->getPdgCode();
        simHit->addContrib(simParticle, pdgCode, edep, time);
    }
}

} // namespace sim
