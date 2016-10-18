#ifndef SimApplication_PrimaryGeneratorMessenger_h
#define SimApplication_PrimaryGeneratorMessenger_h

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

        PrimaryGeneratorAction* primaryGeneratorAction;

        G4UIdirectory* lheDir;
        G4UIcommand* lheOpenCmd;
};

}

#endif
