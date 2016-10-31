#ifndef SIMAPPLICATION_USERTRACKINGACTION_H_
#define SIMAPPLICATION_USERTRACKINGACTION_H_

// Geant4
#include "G4UserTrackingAction.hh"

namespace sim {

class UserTrackingAction : public G4UserTrackingAction {

    public:

        UserTrackingAction();

        virtual ~UserTrackingAction();

    public:

        void PreUserTrackingAction(const G4Track*);

        void PostUserTrackingAction(const G4Track*);
};

}

#endif
