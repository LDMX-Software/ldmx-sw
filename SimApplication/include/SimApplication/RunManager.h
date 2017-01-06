/**
 * @file RunManager.h
 * @brief Class providing a Geant4 run manager implementation
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_RUNMANAGER_H_
#define SIMAPPLICATION_RUNMANAGER_H_

// Geant4
#include "G4RunManager.hh"
#include "G4hMultipleScattering.hh"
#include "G4Decay.hh"

// LDMX
#include "SimPlugins/PluginMessenger.h"

using sim::PluginMessenger;

namespace sim {

/**
 * @class RunManager
 * @brief Extension of Geant4 run manager
 */
class RunManager : public G4RunManager {

    public:

        /**
         * Class constructor.
         */
        RunManager();

        /**
         * Class destructor.
         */
        virtual ~RunManager();

        /**
         * Initialize physics.
         */
        void InitializePhysics();

        /**
         * Perform application initialization.
         */
        void Initialize();

    private:

        /**
         * The plugin messenger.
         */
        PluginMessenger* pluginMessenger_;

        /**
         * Manager of sim plugins.
         */
        PluginManager* pluginManager_;
};

}

#endif
