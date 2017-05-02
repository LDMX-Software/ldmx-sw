/**
 * @file MultiParticleGunPrimaryGenerator.h
 * @brief Class for generating a Geant4 event from LHE event data
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_MULTIPARTICLEGUNPRIMARYGENERATOR_H_
#define SIMAPPLICATION_MULTIPARTICLEGUNPRIMARYGENERATOR_H_

// Geant4
#include "G4VPrimaryGenerator.hh"
#include "TFile.h"
#include "TTree.h"
#include "TRandom.h"
#include "TLorentzVector.h"
#include "TClonesArray.h"
#include "TVector3.h"
#include <fstream>
#include <iostream>

#include "Event/EventHeader.h"
#include "SimApplication/PrimaryGeneratorMessenger.h"

namespace ldmx {

    /**
     * @class MultiParticleGunPrimaryGenerator
     * @brief Generates a Geant4 event from particle gun, but can have many particles
     */
    class MultiParticleGunPrimaryGenerator : public G4VPrimaryGenerator {

        public:

            /**
             * Class constructor.
             * @param reader The LHE reader with the event data.
             */
            MultiParticleGunPrimaryGenerator();

            /**
             * Class destructor.
             */
            virtual ~MultiParticleGunPrimaryGenerator();

            /**
             * Generate vertices in the Geant4 event.
             * @param anEvent The Geant4 event.
             */
            void GeneratePrimaryVertex(G4Event* anEvent);

        private:

            /**
             * The number of particles to generate
             */
            int nParticles_;

            /**
             * The RNG
             */
            TRandom* random_;
    };

}

#endif
