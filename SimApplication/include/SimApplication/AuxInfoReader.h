/**
 * @file AuxInfoReader.h
 * @brief Class for reading reading auxiliary information from GDML
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_AUXINFOREADER_H_
#define SIMAPPLICATION_AUXINFOREADER_H_

// Geant4
#include "G4GDMLParser.hh"

// LDMX
#include "DetDescr/DetectorHeader.h"

namespace sim {

/**
 * @class AuxInfoReader
 * @brief Reads auxiliary information from GDML userinfo block
 *
 * @note
 * This class reads information to define a detector header block, sensitive detectors,
 * visualization attributes, magnetic fields, detector IDs, and detector regions from
 * the userinfo block of a GDML file.  These objects are then assigned to the appropriate
 * logical volumes which have <i>auxiliary</i> tags that reference these objects
 * by name.
 */
class AuxInfoReader {

    public:

        /**
         * Class constructor.
         * @param parser The GDML parser.
         */
        AuxInfoReader(G4GDMLParser* parser);

        /**
         * Class destructor.
         */
        virtual ~AuxInfoReader();

        /**
         * Read the global auxiliary information from the auxinfo block.
         */
        void readGlobalAuxInfo();

        /**
         * Assign auxiliary info to volumes such as sensitive detectors.
         */
        void assignAuxInfoToVolumes();

        /**
         * Get the detector header that was created from the userinfo block.
         * @return The detector header.
         */
        detdescr::DetectorHeader* getDetectorHeader() {
            return detectorHeader_;
        }

    private:

        /**
         * Create a sensitive detector from GDML data.
         * @param sdType The type of the sensitive detector.
         * @param auxInfoList The aux info defining the sensitive detector.
         */
        void createSensitiveDetector(G4String sdType, const G4GDMLAuxListType* auxInfoList);

        /**
         * Create a detector ID from GDML data.
         * @param name The name of the detector ID.
         * @param auxInfoList The aux info defining the detector ID.
         */
        void createDetectorID(G4String name, const G4GDMLAuxListType* auxInfoList);

        /**
         * Create a magnetic field from GDML data.
         * @param name The name of the magnetic field.
         * @param auxInfoList The aux info defining the magnetic field.
         */
        void createMagneticField(G4String name, const G4GDMLAuxListType* auxInfoList);

        /**
         * Create a detector region from GDML data.
         * @param name The name of the detector region.
         * @param auxInfoList The aux info defining the detector region.
         */
        void createRegion(G4String name, const G4GDMLAuxListType* auxInfoList);

        /**
         * Create visualization attributes from GDML data.
         * @param name The name of the visualization attributes.
         * @param auxInfoList The aux info defining the visualization attributes.
         */
        void createVisAttributes(G4String name, const G4GDMLAuxListType* auxInfoList);

        /**
         * Create the detector header from the global auxinfo.
         * @param name The aux value with the detector version.
         * @param auxInfoList The aux info with the detector header information.
         */
        void createDetectorHeader(G4String detectorVersion, const G4GDMLAuxListType* auxInfoList);

    private:

        /**
         * The GDML parser.
         */
        G4GDMLParser* parser_;

        /**
         * The GDML expression evaluator.
         */
        G4GDMLEvaluator* eval_;

        /**
         * Detector header with name and version.
         */
        detdescr::DetectorHeader* detectorHeader_{nullptr};
};

}

#endif
