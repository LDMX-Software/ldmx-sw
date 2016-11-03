#ifndef SIMAPPLICATION_USERSTEPPINGACTION_H_
#define SIMAPPLICATION_USERSTEPPINGACTION_H_

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
