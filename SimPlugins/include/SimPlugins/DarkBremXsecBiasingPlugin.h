/**
 * @file DarkBremXsecBiasingPlugin.h
 * @brief Class that defines a simulation plugin for biasing the DarkBrem xsec by a specified value
 * @author Michael Revering, University of Minnesota
 */

#ifndef SIMPLUGINS_DarkBremXSECBIASINGPLUGIN_H_
#define SIMPLUGINS_DarkBremXSECBIASINGPLUGIN_H_

// Geant4
#include "G4Electron.hh"
#include "G4VEnergyLossProcess.hh"
#include "G4RunManager.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessTable.hh"

// LDMX
#include "SimPlugins/UserActionPlugin.h"
#include "SimPlugins/DarkBremXsecBiasingMessenger.h"

// Sim Core
#include "SimCore/G4eDarkBremsstrahlung.h"

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
            DarkBremXsecBiasingPlugin();

            /**
             * Class destructor.
             */
            ~DarkBremXsecBiasingPlugin();

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
             * Sets biasing factor for PN reactions.
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
	     * Resets the process to active, to limit the number of brems per event to one.
	     * @param aEvent The Geant4 event that is ending.
	     */
	    void endEvent(const G4Event* aEvent);

            /**
             * Set the PN cross-section biasing factor.
             * @param xsecBiasingFactor The new xsec biasing factor.
             */
            void setXsecBiasingFactor(double xsecBiasingFactor) {
                xsecBiasingFactor_ = xsecBiasingFactor;
            }

	    /**
	     * Set the simulation mode.
	     * @param mode The new simulation mode.
	     */
	    void setXsecSimulationMode(std::string mode) {
	        mode_ = mode;
            }

        private:

            /** DarkBrem cross-section multiplicative factor. */
            double xsecBiasingFactor_ {1000};

            /** DarkBrem simulation mode ("forward_only" or "cm_scaling") */
            std::string mode_ {"forward_only"};

            /** Messenger used to parse arguments specified in a macro. */
            DarkBremXsecBiasingMessenger* messenger_ {new DarkBremXsecBiasingMessenger {this}};
    };

}

#endif // SIMPLUGINS_DarkBremXSECBIASINGPLUGIN_H__
