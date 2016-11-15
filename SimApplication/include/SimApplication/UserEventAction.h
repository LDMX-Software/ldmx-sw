#ifndef SIMAPPLICATION_USEREVENTACTION_H_
#define SIMAPPLICATION_USEREVENTACTION_H_

// LDMX
#include "SimApplication/SimParticleBuilder.h"
#include "SimPlugins/PluginManagerAccessor.h"

// Geant4
#include "G4UserEventAction.hh"
#include "G4Event.hh"

namespace sim {

class UserEventAction:
        public G4UserEventAction,
        public PluginManagerAccessor {

    public:

        UserEventAction() {;}
        virtual ~UserEventAction() {;}
        void BeginOfEventAction(const G4Event*);
        void EndOfEventAction(const G4Event*);
};

}

#endif
