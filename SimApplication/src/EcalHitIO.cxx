#include "SimApplication/EcalHitIO.h"

// STL
#include <map>

// LDMX
#include "Event/SimCalorimeterHit.h"
#include "Event/SimParticle.h"

namespace ldmx {

    void EcalHitIO::writeHitsCollection(G4CalorimeterHitsCollection* hc, std::vector<SimCalorimeterHit> &outputColl) {

        int nHits = hc->GetSize();
        std::map< int, SimCalorimeterHit > hitMap;

        // Loop over input hits from Geant4.
        for (int iHit = 0; iHit < nHits; iHit++) {

            // Get the hit and its ID.
            G4CalorimeterHit* g4hit = (G4CalorimeterHit*) hc->GetHit(iHit);
            int hitID = g4hit->getID();

            // See if hit exists in map already.
            std::map<int, SimCalorimeterHit >::iterator it = hitMap.find(hitID);

            // Is it a new hit?
            if (it == hitMap.end()) {

                // Create sim hit and assign the ID.
                hitMap[hitID] = SimCalorimeterHit();
                hitMap[hitID].setID(hitID);

                /**
                 * Assign XY position to the hit using the ECal hex readout.
                 * Z position is set from the original hit, which should be the middle of the sensor.
                 */
                detID_.setRawValue(hitID);
                detID_.unpack();
                int cellID = detID_.getFieldValue("cell");
                int moduleID = detID_.getFieldValue("module_position");
                int cellModuleID = hexReadout_.combineID(cellID,moduleID);
                std::pair<double,double> XYPair = hexReadout_.getCellCenterAbsolute(cellModuleID);
                hitMap[hitID].setPosition(XYPair.first, XYPair.second, g4hit->getPosition().z());

            } 

            // Get info from the G4 hit.
            float edep = g4hit->getEdep();
            float time = g4hit->getTime();
            int pdgCode = g4hit->getPdgCode();

            // Is hit contrib output enabled?
            if (enableHitContribs_) {

                // Find the SimParticle associated with this hit.
                int trackID = g4hit->getTrackID();

                // Find if there is an existing hit contrib.
                int contribIndex = hitMap[hitID].findContribIndex(trackID, pdgCode);

                // Is contrib output being compressed and a record exists for this SimParticle and PDG code?
                if (compressHitContribs_ && contribIndex != -1) {

                    // Update an existing hit contrib.
                    hitMap[hitID].updateContrib(contribIndex, edep, time);

                } else {

                    // Add a hit contrib because all steps are being saved or there is not an existing record.
                    hitMap[hitID].addContrib(trackID, pdgCode, edep, time);

                }

            } else {

                // Hit contributions are not being saved so manually increment the edep and set time.
                hitMap[hitID].setEdep(hitMap[hitID].getEdep() + edep);
                if (time < hitMap[hitID].getTime() || hitMap[hitID].getTime() == 0) {
                    hitMap[hitID].setTime(time);
                }

            } //contrib output enabled or not
        }//loop through geant4 hits

        //copy aggregated hits into output collection
        outputColl.clear();
        for ( auto &mapHit : hitMap ) {
            outputColl.push_back( mapHit.second );
        }

        return;
    }

} // namespace sim
