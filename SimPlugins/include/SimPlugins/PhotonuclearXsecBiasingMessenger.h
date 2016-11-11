#ifndef SIMPLUGINS_PHOTONUCLEARXSECBIASINGPLUGINMESSENGER_H
#define SIMPLUGINS_PHOTONUCLEARXSECBIASINGPLUGINMESSENGER_H

#include "SimPlugins/UserActionPluginMessenger.h"

namespace sim {

// Forward declare to avoid circular dependency in headers.
class PhotonuclearXsecBiasingPlugin;

class PhotonuclearXsecBiasingMessenger : UserActionPluginMessenger {

    public:

        PhotonuclearXsecBiasingMessenger(PhotonuclearXsecBiasingPlugin*);

        virtual ~PhotonuclearXsecBiasingMessenger() {
            delete xsecFactorCmd_;
        }

        void SetNewValue(G4UIcommand *command, G4String newValue);

    private:

        PhotonuclearXsecBiasingPlugin* biasingPlugin_;
        G4UIcommand* xsecFactorCmd_;
};

}

#endif
