/**
 * @file EcalHitIO.h
 * @brief Class providing hit readout for simulated LDMX ECal detector
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

/**
 * @class EcalHitIO
 * @brief Provides hit readout for simulated ECal detector
 *
 * @note
 * This class uses the EcalHexReadout to transform the G4CalorimeterHit hits collection
 * into a collection of SimCalorimeterHit objects assigned to hexagonal cells by their ID
 * and positions.  Energy depositions in the same cells are combined together into single
 * hits.
 *
 * @par
 * It can be run in three modes:
 * <ul>
 * <li>Compressed hit contributions combined by matching SimParticle and PDG code (default mode)
 * <li>Full hit contributions with one record per step (when compressHitContribs_ is false)
 * <li>No hit contribution information where energy is combined but vectors are not filled (when enableHitContribs_ is false)
 * </ul>
 */
class EcalHitIO {

    public:

        /**
         * Layer-cell pair.
         */
        typedef std::pair<int, int> LayerCellPair;

    public:

        /**
         * Class constructor.
         * @param simParticleBuilder Object for accessing SimParticle store for the event.
         */
        EcalHitIO(SimParticleBuilder* simParticleBuilder);

        /**
         * Class destructor.
         */
        ~EcalHitIO() {;}

        /**
         * Write out a Geant4 hits collection to the provided ROOT array.
         * @param hc The input hits collection.
         * @param outputColl The output collection in ROOT.
         */
        void writeHitsCollection(G4CalorimeterHitsCollection* hc, TClonesArray* outputColl);

        /**
         * Set whether hit contributions should be enabled for the output hits.
         * @param enableHitContribs True to enable hit contributions.
         */
        void setEnableHitContribs(bool enableHitContribs) {
            enableHitContribs_ = enableHitContribs;
        }

        /**
         * Set whether hit contributions should be compressed by particle and PDG code.
         * @param compressHitContribs True to compress hit contribution information.
         */
        void setCompressHitContribs(bool compressHitContribs) {
            compressHitContribs_ = compressHitContribs;
        }

    private:

        /**
         * Access to SimParticle list.
         */
        SimParticleBuilder* simParticleBuilder_;

        /**
         * Hex cell readout.
         */
        EcalHexReadout hexReadout_;

        /**
         * ECal detector ID.
         */
        EcalDetectorID detID_;

        /**
         * Enable hit contribution output.
         */
        bool enableHitContribs_{true};

        /**
         * Enable compression of hit contributions by SimParticle and PDG code.
         */
        bool compressHitContribs_{true};
};

} // namespace sim


#endif
