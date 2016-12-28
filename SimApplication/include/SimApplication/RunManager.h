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

        void enableBiasing() { enableBiasing_ = true; }

        void setParticleTypeToBias(std::string particleTypeToBias) { particleTypeToBias_ = particleTypeToBias;}

    private:

        PluginMessenger* pluginMessenger_;
        PluginManager* pluginManager_;

        /** Flag indicating whether physics biasing is enabled or disabled */
        bool enableBiasing_{false};

        /** Particle type to bias. */
        std::string particleTypeToBias_{"gamma"};
};

}

#endif
