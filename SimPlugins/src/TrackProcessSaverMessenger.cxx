#include "SimPlugins/TrackProcessSaverMessenger.h"

#include "SimPlugins/TrackProcessSaver.h"

namespace ldmx {

    TrackProcessSaverMessenger::TrackProcessSaverMessenger(TrackProcessSaver* plugin)
                : UserActionPluginMessenger(plugin), plugin_(plugin) {

        addProcessCmd_ = new G4UIcommand(std::string(getPath() + "addProcess").c_str(), this);
        G4UIparameter* processName = new G4UIparameter("processName", 's', false);
        addProcessCmd_->SetParameter(processName);
        addProcessCmd_->SetGuidance("Add a creator physics process name for saving tracks.");
        addProcessCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);
    }

    TrackProcessSaverMessenger::~TrackProcessSaverMessenger() {
    }

    void TrackProcessSaverMessenger::SetNewValue(G4UIcommand *command, G4String newValue) {

        // Handles verbose command.
        UserActionPluginMessenger::SetNewValue(command, newValue);

        if (command == addProcessCmd_) {
            plugin_->addProcess(newValue);
        }
    }
}
