#ifndef SIMAPPLICATION_USEREVENTACTION_H_
#define SIMAPPLICATION_USEREVENTACTION_H_ 1

// Geant4
#include "G4UserEventAction.hh"
#include "G4Event.hh"

class UserEventAction: public G4UserEventAction {

    public:

        UserEventAction();
        virtual ~UserEventAction();
        void BeginOfEventAction(const G4Event*);
        void EndOfEventAction(const G4Event*);
};

#endif
