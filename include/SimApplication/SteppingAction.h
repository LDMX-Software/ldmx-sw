#ifndef SIMAPPLICATION_USERSTEPPINGACTION_H_
#define SIMAPPLICATION_USERSTEPPINGACTION_H_

// LDMX
#include "SimPlugins/PluginManagerAccessor.h"

// Geant4
#include "G4UserSteppingAction.hh"

namespace sim {

class SteppingAction :
        public G4UserSteppingAction,
        public PluginManagerAccessor {

    public:

        virtual ~SteppingAction() {;}

        void UserSteppingAction(const G4Step*);
};


}

#endif
