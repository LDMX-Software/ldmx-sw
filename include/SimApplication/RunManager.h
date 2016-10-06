#ifndef SIMAPPLICATION_RUNMANAGER_H_
#define SIMAPPLICATION_RUNMANAGER_H_ 1

// Geant4
#include "G4RunManager.hh"
#include "G4hMultipleScattering.hh"
#include "G4Decay.hh"

class RunManager : public G4RunManager {

    public:

        RunManager();

        virtual ~RunManager();

        void InitializePhysics();

        void Initialize();
};

#endif
