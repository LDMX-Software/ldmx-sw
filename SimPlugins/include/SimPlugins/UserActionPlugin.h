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
 * An implementation class should provide concrete implementations of all
 * methods and must define "create" and "destroy" functions as entry points for
 * the dynamic library loading.  The <i>DummySimPlugin</i> class provides an
 * example implementation showing how to define these functions properly.
 *
 * @see DummySimPlugin
 */
class UserActionPlugin {

    public:

        virtual ~UserActionPlugin() {;}

        virtual std::string getName() = 0;

        virtual bool hasRunAction() = 0;

        virtual bool hasSteppingAction() = 0;

        virtual bool hasTrackingAction() = 0;

        virtual bool hasEventAction() = 0;

        virtual void beginRun(const G4Run*) = 0;

        virtual void endRun(const G4Run*) = 0;

        virtual void stepping(const G4Step*) = 0;

        virtual void preTracking(const G4Track*) = 0;

        virtual void postTracking(const G4Track*) = 0;

        virtual void beginEvent(const G4Event*) = 0;

        virtual void endEvent(const G4Event*) = 0;
};

}

#endif
