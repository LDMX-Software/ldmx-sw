/**
 * @file MultiParticleGunPrimaryGenerator.h
 * @brief Class for generating an event using multiple particles.
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Nhan Tran, FNAL
 */

#ifndef SIMAPPLICATION_MULTIPARTICLEGUNPRIMARYGENERATOR_H_
#define SIMAPPLICATION_MULTIPARTICLEGUNPRIMARYGENERATOR_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <fstream>
#include <iostream>
#include <string>

//------------//
//   Geant4   //
//------------//
#include "G4Event.hh"
#include "G4IonTable.hh"
#include "G4PhysicalConstants.hh"
#include "G4VPrimaryGenerator.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"

//----------//
//   ROOT   //
//----------//
#include "TFile.h"
#include "TLorentzVector.h"
#include "TRandom.h"
#include "TTree.h"
#include "TVector3.h"

//-------------//
//   LDMX-SW   //
//-------------//
#include "Event/EventConstants.h"
#include "Event/EventHeader.h"
#include "SimApplication/PrimaryGeneratorMessenger.h"
#include "SimApplication/UserPrimaryParticleInformation.h"

namespace ldmx {

    /**
     * @class MultiParticleGunPrimaryGenerator
     * @brief Generates a Geant4 event from particle gun, but can have many particles
     */
    class MultiParticleGunPrimaryGenerator : public G4VPrimaryGenerator {

        public:

            /** Constructor */
            MultiParticleGunPrimaryGenerator();

            /** Destructor */
            virtual ~MultiParticleGunPrimaryGenerator();

            /**
             * Generate vertices in the Geant4 event.
             *
             * @param anEvent The Geant4 event.
             */
            void GeneratePrimaryVertex(G4Event* anEvent);

            /** 
             * When enabled, the number of incident particles is Poisson 
             * distributed.
             */
            void enablePoisson(){ mpgEnablePoisson_ = true; }

            /** Set the PDG ID of the particle to be used by this gun. */
            void setMpgPdgId( int iPdgid ){ mpgPdgID_ = iPdgid; }

            /** Set the number of particles. */
            void setMpgNparticles( double iNPar ){ mpgNParticles_ = iNPar; }

            /** 
             * Set the vertex position from which to fire the particles from.
             *
             * @param Three vector containing the vertex position of the 
             *        particles.
             */
            void setMpgVertex( G4ThreeVector iVert ){ mpgVertex_ = iVert; }

            /** 
             * Set the initial momentum of the particles.
             *
             * @param Three vector containing the initial momentum of the 
             *        particle.
             */
            void setMpgMomentum( G4ThreeVector iMom ){ mpgMomentum_ = iMom; }

        private:        
            
            /** Random number generator. */
            TRandom* random_;

            /** The vertex position from which to fire the particles. */
            G4ThreeVector mpgVertex_;

            /** The initial momentum of the particles. */
            G4ThreeVector mpgMomentum_;
            
            /** Number of particles that will be fired by the gun per event. */
            double mpgNParticles_{1.};
            
            /** PDG ID of the particle used by the gun. */
            int mpgPdgID_{99999};

            /** 
             * Flag denoting whether the number of incident particles should
             * be Poisson distributed. 
             */
            bool mpgEnablePoisson_{false};

    }; // MultiParticleGunPrimaryGenerator

} // ldmx 

#endif // SIMAPPLICATION_MULTIPARTICLEGUNPRIMARYGENERATOR_H_
