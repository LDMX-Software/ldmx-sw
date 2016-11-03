#ifndef SIMAPPLICATION_USEREVENTACTION_H_
#define SIMAPPLICATION_USEREVENTACTION_H_

// Geant4
#include "G4UserEventAction.hh"
#include "G4Event.hh"

// LDMX
#include "SimApplication/SimParticleBuilder.h"

namespace sim {

class UserEventAction: public G4UserEventAction {

    public:

        UserEventAction();
        virtual ~UserEventAction();
        void BeginOfEventAction(const G4Event*);
        void EndOfEventAction(const G4Event*);

    private:

        SimParticleBuilder* simParticleBuilder;
};

}

#endif
