/**
 * @file RootPrimaryGenerator.h
 * @brief Class for generating a Geant4 event from LHE event data
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_ROOTPRIMARYGENERATOR_H_
#define SIMAPPLICATION_ROOTPRIMARYGENERATOR_H_

// Geant4
#include "G4VPrimaryGenerator.hh"


namespace ldmx {

/**
 * @class RootPrimaryGenerator
 * @brief Generates a Geant4 event from an LHEEvent
 */
class RootPrimaryGenerator : public G4VPrimaryGenerator {

    public:

        /**
         * Class constructor.
         * @param reader The LHE reader with the event data.
         */
        RootPrimaryGenerator( G4String filename );

        /**
         * Class destructor.
         */
        virtual ~RootPrimaryGenerator();

        /**
         * Generate vertices in the Geant4 event.
         * @param anEvent The Geant4 event.
         */
        void GeneratePrimaryVertex(G4Event* anEvent);

    private:

        /**
         * The LHE reader with the event data.
         */
        G4String filename_;
};

}

#endif
