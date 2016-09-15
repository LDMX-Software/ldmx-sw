#ifndef SIMAPPLICATION_USERRUNACTION_H_
#define SIMAPPLICATION_USERRUNACTION_H_ 1

#include "G4UserRunAction.hh"

class UserRunAction : public G4UserRunAction {

public:
    UserRunAction();
    virtual ~UserRunAction();

    void BeginOfRunAction(const G4Run*);
    void EndOfRunAction(const G4Run*);
};

#endif
