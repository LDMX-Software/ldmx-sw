#ifndef SimApplication_UserSteppingAction_h
#define SimApplication_UserSteppingAction_h

// Geant4
#include "G4UserSteppingAction.hh"

namespace sim {

class SteppingAction : public G4UserSteppingAction {

    public:

        virtual ~SteppingAction() {;}

        void UserSteppingAction(const G4Step*);
};


}

#endif
