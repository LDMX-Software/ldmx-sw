#ifndef SIMPLUGINS_USERACTIONPLUGIN_H_
#define SIMPLUGINS_USERACTIONPLUGIN_H_

#include "G4Run.hh"
#include "G4Event.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4ClassificationOfNewTrack.hh"

namespace sim {

/**
 * @class UserActionPlugin
 * @brief User simulation plugin
 *
 * This class defines a plugin interface to the Geant4 simulation engine
 * which is activated in the "user action" hooks.  An implementation
 * class must define "create" and "destroy" functions as entry points for
 * the dynamic library loading.  The <i>DummySimPlugin</i> class provides
 * an example implementation showing how to define these functions properly.
 * The user's plugin need only override the functions for which it is
 * implementing plugin actions.  By default, the plugin does not activate
 * any of the user actions.
 *
 * @see DummySimPlugin
 */
class UserActionPlugin {

    public:

        virtual ~UserActionPlugin() {;}

        virtual std::string getName() = 0;

        void setVerboseLevel(int verbose) {
            verbose_ = verbose;
            if (verbose_ < 1) {
                verbose = 1;
            } else if (verbose_ > 4) {
                verbose = 4;
            }
        }

        int getVerboseLevel() {
            return verbose_;
        }

        virtual bool hasRunAction() { return false; }

        virtual bool hasSteppingAction() { return false; }

        virtual bool hasTrackingAction() { return false; }

        virtual bool hasEventAction() { return false; }

        virtual bool hasStackingAction() { return false; }

        virtual bool hasPrimaryGeneratorAction() { return false; }

        virtual void beginRun(const G4Run*) {;}

        virtual void endRun(const G4Run*) {;}

        virtual void stepping(const G4Step*) {;}

        virtual void preTracking(const G4Track*) {;}

        virtual void postTracking(const G4Track*) {;}

        virtual void beginEvent(const G4Event*) {;}

        virtual void endEvent(const G4Event*) {;}

        virtual void generatePrimary(G4Event*) {;}

        /**
         * @brief
         * Classify a new track.
         *
         * @note
         * The default behavior is to return the current classification which will be <i>fUrgent</i> if no other plugin changed it.
         */
        virtual G4ClassificationOfNewTrack stackingClassifyNewTrack(const G4Track*, const G4ClassificationOfNewTrack& currentTrackClass) {
            return currentTrackClass;
        }

        virtual void stackingNewStage() {;}

        virtual void stackingPrepareNewEvent() {;}

    protected:

        // Sub-classes can access the verbose level directly for convenience.
        int verbose_{1};
};

}

#endif
