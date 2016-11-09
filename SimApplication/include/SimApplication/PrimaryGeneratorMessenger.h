#ifndef SIMAPPLICATION_PRIMARYGENERATORMESSENGER_H_
#define SIMAPPLICATION_PRIMARYGENERATORMESSENGER_H_

// Geant4
#include "G4UImessenger.hh"

// LDMX
#include "SimApplication/PrimaryGeneratorAction.h"

namespace sim {

class PrimaryGeneratorMessenger : public G4UImessenger {

    public:

        PrimaryGeneratorMessenger(PrimaryGeneratorAction*);

        virtual ~PrimaryGeneratorMessenger();

        void SetNewValue(G4UIcommand* command, G4String newValues);

    private:

        PrimaryGeneratorAction* primaryGeneratorAction_;

        G4UIdirectory* lheDir_;
        G4UIcommand* lheOpenCmd_;
};

}

#endif
