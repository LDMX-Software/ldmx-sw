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

class RunManager : public G4RunManager {

    public:

        RunManager();

        virtual ~RunManager();

        void InitializePhysics();

        void Initialize();

    private:

        PluginMessenger* pluginMessenger_;
        PluginManager* pluginManager_;
};

}

#endif
