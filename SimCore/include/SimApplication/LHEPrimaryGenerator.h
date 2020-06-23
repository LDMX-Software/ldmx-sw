/**
 * @file LHEPrimaryGenerator.h
 * @brief Class for generating a Geant4 event from LHE event data
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMAPPLICATION_LHEPRIMARYGENERATOR_H
#define SIMAPPLICATION_LHEPRIMARYGENERATOR_H

// LDMX
#include "SimApplication/PrimaryGenerator.h"
#include "SimApplication/LHEReader.h"

class G4Event;

namespace ldmx {

    class Parameters;

    /**
     * @class LHEPrimaryGenerator
     * @brief Generates a Geant4 event from an LHEEvent
     */
    class LHEPrimaryGenerator : public PrimaryGenerator {

        public:

            /**
             * Class constructor.
             * @param reader The LHE reader with the event data.
             */
            LHEPrimaryGenerator(const std::string& name, Parameters& parameters);

            /**
             * Class destructor.
             */
            virtual ~LHEPrimaryGenerator();

            /**
             * Generate vertices in the Geant4 event.
             * @param anEvent The Geant4 event.
             */
            void GeneratePrimaryVertex(G4Event* anEvent);

        private:

            /**
             * The LHE reader with the event data.
             */
            LHEReader* reader_;
    };

}

#endif // SIMAPPLICATION_LHEPRIMARYGENERATOR_H
