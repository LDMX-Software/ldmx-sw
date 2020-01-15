/**
 * @file DarkBremXsecBiasingPlugin.h
 * @brief Class that defines a simulation plugin for biasing the DarkBrem xsec by a specified value
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMPLUGINS_DARKBREMXSECBIASINGPLUGIN_H_
#define SIMPLUGINS_DARKBREMXSECBIASINGPLUGIN_H_

// LDMX
#include "SimCore/G4eDarkBremsstrahlung.h"
#include "SimPlugins/UserActionPlugin.h"
#include "SimPlugins/DarkBremXsecBiasingMessenger.h"

namespace ldmx {

    /**
     * @class DarkBremXsecBiasingPlugin
     * @brief User sim plugin to bias the Dark Brem cross-section by a specified value
     */
    class DarkBremXsecBiasingPlugin : public UserActionPlugin {

        public:

            /**
             * Class constructor.
             */
            DarkBremXsecBiasingPlugin() : UserActionPlugin() { }

            /**
             * Class destructor.
             *
             * Deletes the messenger attached to this plugin.
             */
            ~DarkBremXsecBiasingPlugin() {
                delete messenger_;
            }

            /**
             * Get the name of the plugin.
             * @return The name of the plugin.
             */
            virtual std::string getName() {
                return "DarkBremXsecBiasingPlugin";
            }

            /**
             * Get whether this plugin implements a run action.
             * @return True to indicate this plugin has a run action.
             */
            bool hasRunAction() {
                return true;
            }

            /**
             * Implementation of begin run hook.
             * 
             * Gets the dark brem process and passes the setup variables for initialisation.
             *
             * @param aRun The Geant4 run that is starting.
             */
            void beginRun(const G4Run* aRun);

            /**
             * Get whether this plugin implements an event action.
             * @return True to indicate this plugin has an event action.
             */
            bool hasEventAction() {
                return true;
            }

            /**
             * Implementation of end event hook.
             *
             * Resets the process to active.
             * The process is deactivated every time it occurs to limit it to once brem per event.
             *
             * @param aEvent The Geant4 event that is ending.
             */
            void endEvent(const G4Event* aEvent);

            /**
             * Set the dark brem cross-section biasing factor.
             * @param xsecBiasingFactor The new xsec biasing factor.
             */
            void setXsecBiasingFactor(double xsecBiasingFactor) {
                xsecBiasingFactor_ = xsecBiasingFactor;
            }

            /**
             * Set the simulation mode.
             * @param mode The new simulation mode.
             */
            void setXsecSimulationMethod(G4eDarkBremsstrahlungModel::DarkBremMethod method) {
                method_ = method;
            }

            /**
             * Set the mad graph data file.
             * @param path the path to the data file
             */
            void setMadGraphDataFile(std::string path) {
                madGraphDataFile_ = path;
            }

        private:

            /** DarkBrem cross-section multiplicative factor. */
            double xsecBiasingFactor_ {1};

            /** DarkBrem simulation mode ("forward_only" or "cm_scaling") */
            G4eDarkBremsstrahlungModel::DarkBremMethod method_{G4eDarkBremsstrahlungModel::DarkBremMethod::Undefined};

            /** Mad Graph data file containing dark brem events */
            std::string madGraphDataFile_{""};

            /** Messenger used to parse arguments specified in a macro. */
            DarkBremXsecBiasingMessenger* messenger_ {new DarkBremXsecBiasingMessenger {this}};
    };

}

#endif // SIMPLUGINS_DARKBREMXSECBIASINGPLUGIN_H_
