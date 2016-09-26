#ifndef SIMAPPLICATION_USERTRACKINGACTION_HH_
#define SIMAPPLICATION_USERTRACKINGACTION_HH_ 1

// Geant4
#include "G4UserTrackingAction.hh"

class UserTrackingAction : public G4UserTrackingAction {

    public:

        UserTrackingAction();

        virtual ~UserTrackingAction();

    public:

        void PreUserTrackingAction(const G4Track*);

        void PostUserTrackingAction(const G4Track*);
};

#endif
