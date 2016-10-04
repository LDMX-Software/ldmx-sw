#ifndef SIMAPPLICATION_PRIMARYGENERATORMESSENGER_H_
#define SIMAPPLICATION_PRIMARYGENERATORMESSENGER_H_ 1

// Geant4
#include "G4UImessenger.hh"

// LDMX
#include "SimApplication/PrimaryGeneratorAction.h"

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

#endif
