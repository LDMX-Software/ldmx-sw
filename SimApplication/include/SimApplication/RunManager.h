#ifndef SimApplication_RunManager_h
#define SimApplication_RunManager_h

// Geant4
#include "G4RunManager.hh"
#include "G4hMultipleScattering.hh"
#include "G4Decay.hh"

namespace sim {

class RunManager : public G4RunManager {

    public:

        RunManager();

        virtual ~RunManager();

        void InitializePhysics();

        void Initialize();
};

}

#endif
