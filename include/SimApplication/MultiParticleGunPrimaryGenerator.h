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
#include <string>

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

            void enablePoisson() { mpg_enablePoisson_ = true; }
            void setMpgPdgId( int iPdgid ) { mpg_pdgId_ = iPdgid; }
            void setMpgNparticles( double iNPar ) { mpg_nparticles_ = iNPar; }
            void setMpgVertex( G4ThreeVector iVert ) { mpg_vertex_ = iVert; }
            void setMpgMomentum( G4ThreeVector iMom ) { mpg_momentum_ = iMom; }

        private:        

            /**
             * The RNG
             */
            TRandom* random_;

            bool mpg_enablePoisson_;
            int mpg_pdgId_;
            double mpg_nparticles_;
            G4ThreeVector mpg_vertex_;
            G4ThreeVector mpg_momentum_;
    };

}

#endif
