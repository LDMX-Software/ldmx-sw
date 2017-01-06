/**
 * @file AuxInfoReader.h
 * @brief Class for reading reading auxiliary information from GDML
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_AUXINFOREADER_H_
#define SIMAPPLICATION_AUXINFOREADER_H_

// Geant4
#include "G4GDMLParser.hh"

namespace sim {

/**
 * @class AuxInfoReader
 * @brief Reads auxiliary information from GDML auxinfo block
 *
 * @note
 * This class reads information to define sensitive detectors, visualization attributes,
 * magnetic fields, detector IDs, and detector regions from the auxinfo block of a GDML
 * file.  After this information is read, it can assign the objects to the appropriate
 * logical volumes where there is an <i>auxiliary</i> tag that references the information
 * by its name, using the <i>auxtype<i> to define the type of object being referenced
 * and <i>auxvalue</i> as the name of the referenced object from the <i>auxinfo</i>
 * section.
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

    private:

        /**
         * The GDML parser.
         */
        G4GDMLParser* parser_;

        /**
         * The GDML expression evaluator.
         */
        G4GDMLEvaluator* eval_;
};

}

#endif
