#ifndef SimPlugins_UserActionPlugin_h
#define SimPlugins_UserActionPlugin_h

#include "G4Run.hh"
#include "G4Event.hh"
#include "G4Track.hh"
#include "G4Step.hh"

namespace sim {

class UserActionPlugin {

    public:

        virtual ~UserActionPlugin() {;}

        virtual bool hasRunAction() = 0;

        virtual bool hasSteppingAction() = 0;

        virtual bool hasTrackingAction() = 0;

        virtual bool hasEventAction() = 0;

        virtual void beginOfRunAction(G4Run*) = 0;

        virtual void endOfRunAction(G4Run*) = 0;

        virtual void steppingAction(G4Step*) = 0;

        virtual void preTrackingAction(G4Track*) = 0;

        virtual void postTrackingAction(G4Track*) = 0;

        virtual void beginOfEventAction(G4Event*) = 0;

        virtual void endOfEventAction(G4Event*) = 0;
};

}

#endif
