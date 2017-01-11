/**
 * @file DetectorConstruction.h
 * @brief Class implementing the Geant4 detector construction
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_DETECTORCONSTRUCTION_H_
#define SIMAPPLICATION_DETECTORCONSTRUCTION_H_

// LDMX
#include "AuxInfoReader.h"

// Geant4
#include "G4VUserDetectorConstruction.hh"
#include "G4GDMLParser.hh"

namespace sim {

/**
 * @class DetectorConstruction
 * @brief Implements the Geant4 detector construction
 *
 * @note
 * This class reads in a detector description from a GDML file
 * using the basic <i>G4GDMLParser</i> and instantiates supplemental
 * information using the AuxInfoReader.
 *
 * @see AuxInfoReader
 */
class DetectorConstruction: public G4VUserDetectorConstruction {

    public:

        /**
         * Class constructor.
         * @param theParser GDML parser defining the geometry.
         */
        DetectorConstruction(G4GDMLParser* theParser);

        /**
         * Class destructor.
         */
        virtual ~DetectorConstruction();

        /**
         * Construct the detector.
         * @return The top volume of the detector.
         */
        G4VPhysicalVolume *Construct();

        /**
         * Get the detector header.
         * @return The detector header.
         */
        detdescr::DetectorHeader* getDetectorHeader() {
            return auxInfoReader_->getDetectorHeader();
        }

    private:

        /**
         * The GDML parser defining the detector.
         */
        G4GDMLParser* parser_;

        /**
         * The auxiliary GDML info reader.
         */
        AuxInfoReader* auxInfoReader_;
};

}

#endif
