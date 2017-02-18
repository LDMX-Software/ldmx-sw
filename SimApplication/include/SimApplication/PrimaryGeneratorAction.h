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

namespace ldmx {

/**
 * @class PrimaryGeneratorAction
 * @brief Implementation of Geant4 primary generator action
 */
class PrimaryGeneratorAction :
        public G4VUserPrimaryGeneratorAction,
        public PluginManagerAccessor {

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

    private:

        /**
         * The primary generator.
         */
        G4VPrimaryGenerator* generator_;
};

}

#endif
