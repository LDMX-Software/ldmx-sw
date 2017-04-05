#include "SimPlugins/PhotonuclearXsecBiasingMessenger.h"

// LDMX
#include "SimPlugins/PhotonuclearXsecBiasingPlugin.h"

// STL
#include <string>

namespace ldmx {

    PhotonuclearXsecBiasingMessenger::PhotonuclearXsecBiasingMessenger(PhotonuclearXsecBiasingPlugin* biasingPlugin) :
            UserActionPluginMessenger(biasingPlugin), biasingPlugin_(biasingPlugin) {

        xsecFactorCmd_ = new G4UIcommand(std::string(getPath() + "xsecFactor").c_str(), this);
        G4UIparameter* modulus = new G4UIparameter("xsecFactor", 'd', false);
        xsecFactorCmd_->SetParameter(modulus);
        xsecFactorCmd_->SetGuidance("Set the cross section biasing factor for the photonuclear process (must be >= 100.)");
        xsecFactorCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);
    }

    void PhotonuclearXsecBiasingMessenger::SetNewValue(G4UIcommand *command, G4String newValue) {
        if (command == xsecFactorCmd_) {
            biasingPlugin_->setXsecBiasingFactor(std::stod(newValue));
        }
    }

} // namespace sim
