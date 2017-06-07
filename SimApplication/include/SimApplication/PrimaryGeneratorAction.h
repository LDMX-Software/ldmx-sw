/**
 * @file PrimaryGeneratorAction.h
 * @brief Class implementing the Geant4 primary generator action
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_PRIMARYGENERATORACTION_H_
#define SIMAPPLICATION_PRIMARYGENERATORACTION_H_

// Geant4
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4VPrimaryGenerator.hh"

// LDMX
#include "SimPlugins/PluginManagerAccessor.h"

// RNG
#include "TRandom.h"


namespace ldmx {

    /**
     * @class PrimaryGeneratorAction
     * @brief Implementation of Geant4 primary generator action
     */
    class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction, public PluginManagerAccessor {

        public:

            /**
             * Class constructor.
             */
            PrimaryGeneratorAction();

            /**
             * Class destructor.
             */
            virtual ~PrimaryGeneratorAction();

            /**
             * Generate the event.
             * @param anEvent The Geant4 event.
             */
            virtual void GeneratePrimaries(G4Event* anEvent);

            /**
             * Set the primary generator.
             * @param primaryGenerator The primary generator.
             */
            void setPrimaryGenerator(G4VPrimaryGenerator* primaryGenerator);

            /**
             * Enable beamspot smearing.
             * @param bool
             */
            void setUseBeamspot(bool usebs){ useBeamspot_ = usebs; };

            /**
             * Set beamspot size
             * @param beamspot size
             */
            void setBeamspotXSize(double bssize){ beamspotXSize_ = bssize; };
            void setBeamspotYSize(double bssize){ beamspotYSize_ = bssize; };

            // G4VPrimaryGenerator* getPrimaryGenerator(){ return generator_; };

            G4VPrimaryGenerator* getGenerator(int i){ return generator_.at(i); }

            int getIndexMPG(){ return index_mpg_; }

        private:

            /**
             * Smearing beamspot
             * @param anEvent The Geant4 event.
             */
            void smearingBeamspot(G4Event* anEvent);

            /**
             * The primary generator.
             */
            std::vector< G4VPrimaryGenerator* > generator_;

            /**
             * The RNG
             */
            TRandom* random_;

            // * Particle energy threshold. 
            bool useBeamspot_;
            
            // * Particle energy threshold. 
            double beamspotXSize_;            
            double beamspotYSize_;   

            //
            int index_mpg_;          

    };

}

#endif
