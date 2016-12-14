/**
 * @file EcalHitIO.h
 * @brief Provides hit readout for simulated LDMX ECal detector.
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Owen Colegro, UCSB
 */

#ifndef SIMAPPLICATION_ECALHITIO_H_
#define SIMAPPLICATION_ECALHITIO_H_

// LDMX
#include "DetDescr/EcalDetectorID.h"
#include "DetDescr/EcalHexReadout.h"
#include "Event/SimCalorimeterHit.h"
#include "SimApplication/G4CalorimeterHit.h"
#include "SimApplication/SimParticleBuilder.h"

// ROOT
#include "TClonesArray.h"

// STL
#include <utility>

using detdescr::EcalDetectorID;
using detdescr::EcalHexReadout;
using event::SimCalorimeterHit;
using sim::G4CalorimeterHitsCollection;

namespace sim {

class EcalHitIO {

    public:
        typedef std::pair<int, int> LayerCellPair;

    public:

        /**
         * Write out a Geant4 hits collection to the provided ROOT array.
         * @param hc The input hits collection.
         * @param outputColl The output collection in ROOT.
         * @param simParticleBuilder The sim particle builder for getting sim particles from track ID.
         */
        void writeHitsCollection(G4CalorimeterHitsCollection* hc,
                TClonesArray* outputColl,
                SimParticleBuilder* simParticleBuilder);

    private:

        EcalHexReadout* hexReadout = new EcalHexReadout();
        EcalDetectorID detID;

};

} // namespace sim


#endif
