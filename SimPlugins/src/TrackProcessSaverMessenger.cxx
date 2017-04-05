#include "SimPlugins/TrackProcessSaverMessenger.h"

// LDMX
#include "SimPlugins/TrackProcessSaver.h"

// Geant4
#include "G4UIcmdWithABool.hh"

// C++
#include <sstream>

namespace ldmx {

    TrackProcessSaverMessenger::TrackProcessSaverMessenger(TrackProcessSaver* plugin) :
            UserActionPluginMessenger(plugin), plugin_(plugin) {

        addProcessCmd_ = new G4UIcommand(std::string(getPath() + "addProcess").c_str(), this);
        addProcessCmd_->SetGuidance("Add a creator physics process name for saving tracks.");
        addProcessCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);

        G4UIparameter* processName = new G4UIparameter("processName", 's', false);
        processName->SetGuidance("Name of Geant4 physics process to save");
        addProcessCmd_->SetParameter(processName);

        G4UIparameter* exactMatch = new G4UIparameter("exactMatch", 'b', true);
        exactMatch->SetGuidance("True if process name match should be exact");
        addProcessCmd_->SetParameter(exactMatch);
    }

    TrackProcessSaverMessenger::~TrackProcessSaverMessenger() {
    }

    void TrackProcessSaverMessenger::SetNewValue(G4UIcommand *command, G4String newValue) {

        // Handles verbose command.
        UserActionPluginMessenger::SetNewValue(command, newValue);

        if (command == addProcessCmd_) {
            std::stringstream ss(newValue);
            std::string processName;
            bool exactMatch = false;
            ss >> processName;
            if (!ss.eof()) {
                std::string exactMatchStr;
                ss >> exactMatchStr;
                exactMatch = G4UIcommand::ConvertToBool(exactMatchStr.c_str());
            }
            plugin_->addProcess(processName, exactMatch);
        }
    }
}
