#ifndef SimPlugins_UserActionPlugin_h
#define SimPlugins_UserActionPlugin_h

#include "G4Run.hh"
#include "G4Event.hh"
#include "G4Track.hh"
#include "G4Step.hh"

namespace sim {

/**
 * @class UserActionPlugin
 * @brief User simulation plugin
 *
 * This class is a pure virtual interface defining a plugin to the Geant4
 * simulation engine which may be activated in the "user action" hooks.
 * An implementation class must define "create" and "destroy" functions
 * as entry points for the dynamic library loading.  The <i>DummySimPlugin</i>
 * class provides an example implementation showing how to define these
 * functions properly.
 *
 * @see DummySimPlugin
 */
class UserActionPlugin {

    public:

        virtual ~UserActionPlugin() {;}

        virtual std::string getName() = 0;

        virtual bool hasRunAction() { return false; }

        virtual bool hasSteppingAction() { return false; }

        virtual bool hasTrackingAction() { return false; }

        virtual bool hasEventAction() { return false; }

        virtual void beginRun(const G4Run*) {;}

        virtual void endRun(const G4Run*) {;}

        virtual void stepping(const G4Step*) {;}

        virtual void preTracking(const G4Track*) {;}

        virtual void postTracking(const G4Track*) {;}

        virtual void beginEvent(const G4Event*) {;}

        virtual void endEvent(const G4Event*) {;}
};

}

#endif
