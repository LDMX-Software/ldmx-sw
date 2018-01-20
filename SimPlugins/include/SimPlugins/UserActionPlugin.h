/**
 * @file UserActionPlugin.h
 * @brief Class defining an interface for a user simulation plugin
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMPLUGINS_USERACTIONPLUGIN_H_
#define SIMPLUGINS_USERACTIONPLUGIN_H_

// Geant4
#include "G4Run.hh"
#include "G4Event.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4ClassificationOfNewTrack.hh"

namespace ldmx {

    /**
     * @class UserActionPlugin
     * @brief User simulation plugin
     *
     * @note
     * This class defines a plugin interface to the Geant4 simulation engine
     * which is activated in the "user action" hooks.  An implementation
     * class must define "create" and "destroy" functions as entry points for
     * the dynamic library loading.  By default, the plugin does not activate
     * any of the user actions, and the user should override the methods
     * which return boolean flags to activate these hooks.
     *
     * @par
     * Here are examples of the static functions that should be defined for
     * creating and destroying the plugin in the class's source file, assuming that
     * the plugin is called %DummySimPlugin.
     *
     * @par
     * Example create function:
     * @snippet extern "C" sim::DummySimPlugin* createDummySimPlugin() { return new sim::DummySimPlugin; }
     *
     * @par
     * Example destroy function:
     * @snippet extern "C" void destroyDummySimPlugin(sim::DummySimPlugin* object) { delete object; }
     */
    class UserActionPlugin {

        public:

            /**
             * Class destructor.
             */
            virtual ~UserActionPlugin() {
                ;
            }

            /**
             * Get the name of the plugin.
             * The user must override this function.
             * @return The name of the plugin.
             */
            virtual std::string getName() = 0;

            /**
             * Set the verbose level of the plugin (1-4).
             * @param verbose The verbose level of the plugin.
             */
            void setVerboseLevel(int verbose) {
                verbose_ = verbose;
                if (verbose_ < 1) {
                    verbose = 1;
                } else if (verbose_ > 4) {
                    verbose = 4;
                }
            }

            /**
             * Get the current verbose level.
             * @return The current verbose level.
             */
            int getVerboseLevel() {
                return verbose_;
            }

            /**
             * Get whether this plugin implements the run action.
             * @return True if the plugin implements the run action.
             */
            virtual bool hasRunAction() {
                return false;
            }

            /**
             * Get whether this plugin implements the stepping action.
             * @return True if the plugin implements the stepping action.
             */
            virtual bool hasSteppingAction() {
                return false;
            }

            /**
             * Get whether this plugin implements the tracking action.
             * @return True if the plugin implements the tracking action.
             */
            virtual bool hasTrackingAction() {
                return false;
            }

            /**
             * Get whether this plugin implements the event action.
             * @return True if the plugin implements the event action.
             */
            virtual bool hasEventAction() {
                return false;
            }

            /**
             * Get whether this plugin implements the stacking action.
             * @return True if the plugin implements the stacking action.
             */
            virtual bool hasStackingAction() {
                return false;
            }

            /**
             * Get whether this plugin implements the primary generator action.
             * @return True if the plugin implements the primary generator action.
             */
            virtual bool hasPrimaryGeneratorAction() {
                return false;
            }

            /**
             * Begin of run action.
             */
            virtual void beginRun(const G4Run*) {
            }

            /**
             * End of run action.
             */
            virtual void endRun(const G4Run*) {
            }

            /**
             * Stepping action.
             */
            virtual void stepping(const G4Step*) {
            }

            /**
             * Pre-tracking action.
             */
            virtual void preTracking(const G4Track*) {
            }

            /**
             * Post-tracking action.
             */
            virtual void postTracking(const G4Track*) {
            }

            /**
             * Begin of event action.
             */
            virtual void beginEvent(const G4Event*) {
            }

            /**
             * End of event action.
             */
            virtual void endEvent(const G4Event*) {
            }

            /**
             * Generate primary action.
             */
            virtual void generatePrimary(G4Event*) {
            }

            /**
             * Classify a new track.
             * @param currentTrackClass The current track classification.
             * @return The current track classification returned by default.
             */
            virtual G4ClassificationOfNewTrack stackingClassifyNewTrack(const G4Track*, const G4ClassificationOfNewTrack& currentTrackClass) {
                return currentTrackClass;
            }

            /**
             * New stacking stage action.
             */
            virtual void stackingNewStage() {
            }

            /**
             * New event stacking action.
             */
            virtual void stackingPrepareNewEvent() {
            }

        protected:

            /** Protected access to verbose level for convenience of sub-classes. */
            int verbose_ {1};
    };
}

/*
 * Macro for defining the create and destroy methods for a sim plugin.
 */
#define SIM_PLUGIN(NS, NAME) \
extern "C" NS::NAME* create ## NAME() { \
    return new NS::NAME; \
} \
extern "C" void destroy ## NAME(NS::NAME* object) { \
    delete object; \
}

#endif
