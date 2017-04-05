/**
 * @file PhotonuclearXsecBiasingPlugin.h
 * @brief Class that defines a simulation plugin for biasing the photonuclear xsec by a specified value
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMPLUGINS_PHOTONUCLEARXSECBIASINGPLUGIN_H_
#define SIMPLUGINS_PHOTONUCLEARXSECBIASINGPLUGIN_H_

// Geant4
#include "G4Gamma.hh"
#include "G4HadronicProcess.hh"
#include "G4RunManager.hh"
#include "G4ProcessManager.hh"

// LDMX
#include "SimPlugins/UserActionPlugin.h"
#include "SimPlugins/PhotonuclearXsecBiasingMessenger.h"

namespace ldmx {

    /**
     * @class PhotonuclearXsecBiasingPlugin
     * @brief User sim plugin to bias the Photonuclear cross-section by a specified value
     */
    class PhotonuclearXsecBiasingPlugin : public UserActionPlugin {

        public:

            /**
             * Class constructor.
             */
            PhotonuclearXsecBiasingPlugin();

            /**
             * Class destructor.
             */
            ~PhotonuclearXsecBiasingPlugin();

            /**
             * Get the name of the plugin.
             * @return The name of the plugin.
             */
            virtual std::string getName() {
                return "PhotonuclearXsecBiasingPlugin";
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
             * Set the PN cross-section biasing factor.
             * @param xsecBiasingFactor The new xsec biasing factor.
             */
            void setXsecBiasingFactor(double xsecBiasingFactor) {
                xsecBiasingFactor_ = xsecBiasingFactor;
            }

        private:

            /** Photonuclear cross-section multiplicative factor. */
            double xsecBiasingFactor_ {1000};

            /** Messenger used to parse arguments specified in a macro. */
            PhotonuclearXsecBiasingMessenger* messenger_ {new PhotonuclearXsecBiasingMessenger {this}};
    };

}

#endif // SIMPLUGINS_PHOTONUCLEARXSECBIASINGPLUGIN_H__
