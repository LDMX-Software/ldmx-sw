/**
 * @file PhotonuclearXsecBiasingMessenger.h
 * @brief Class providing a macro messenger for a PhotonuclearXsecBiasingPlugin
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMPLUGINS_PHOTONUCLEARXSECBIASINGPLUGINMESSENGER_H
#define SIMPLUGINS_PHOTONUCLEARXSECBIASINGPLUGINMESSENGER_H

#include "SimPlugins/UserActionPluginMessenger.h"

namespace ldmx {

// Forward declare to avoid circular dependency in headers.
class PhotonuclearXsecBiasingPlugin;

/**
 * @class PhotonuclearXsecBiasingMessenger
 * @brief Messenger for setting parameters on PhotonuclearXsecBiasingPlugin
 */
class PhotonuclearXsecBiasingMessenger : UserActionPluginMessenger {

    public:

        /**
         * Class constructor.
         * @param plugin The associated PhotonuclearXsecBiasingPlugin object.
         */
        PhotonuclearXsecBiasingMessenger(PhotonuclearXsecBiasingPlugin* plugin);

        /**
         * Class destructor.
         */
        virtual ~PhotonuclearXsecBiasingMessenger() {
            delete xsecFactorCmd_;
        }

        /**
         * Process the macro command.
         * @param[in] command The macro command.
         * @param[in] newValues The argument values.
         */
        void SetNewValue(G4UIcommand *command, G4String newValue);

    private:

        /**
         * The associated PhotonuclearXsecBiasingPlugin object.
         */
        PhotonuclearXsecBiasingPlugin* biasingPlugin_;

        /**
         * The command for setting the cross-section biasing factor.
         */
        G4UIcommand* xsecFactorCmd_;
};

}

#endif
