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
#include "Event/SimCalorimeterHit.h"
#include "SimApplication/G4CalorimeterHit.h"

// ROOT
#include "TClonesArray.h"

// STL
#include <utility>

using detdescr::EcalDetectorID;
using event::SimCalorimeterHit;
using sim::G4CalorimeterHitsCollection;

namespace sim {

class EcalHitIO {

    public:
        typedef std::pair<int, int> LayerCellPair;

    public:

        /**
         * Write out a Geant4 hits collection to the provided ROOT array.
         */
        void writeHitsCollection(G4CalorimeterHitsCollection* hc, TClonesArray* outputColl);

    private:

        /**
         * Make a layer-cell pair from a hit.
         */
        LayerCellPair hitToPair(G4CalorimeterHit* g4hit);

    private:

        EcalDetectorID detID;
        std::map<LayerCellPair, int> ecalReadoutMap;

};

} // namespace sim


#endif
