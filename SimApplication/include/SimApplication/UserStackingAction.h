#ifndef SIMAPPLICATION_USERSTACKINGACTION_H
#define SIMAPPLICATION_USERSTACKINGACTION_H

// LDMX
#include "SimPlugins/PluginManager.h"

// Geant4
#include "G4UserStackingAction.hh"

namespace sim {

class UserStackingAction : public G4UserStackingAction {

    public:

        UserStackingAction() {;}
        virtual ~UserStackingAction() {;}

        G4ClassificationOfNewTrack ClassifyNewTrack (const G4Track *aTrack);
        void NewStage();
        void PrepareNewEvent();
};

}

#endif
