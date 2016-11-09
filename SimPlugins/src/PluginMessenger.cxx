#include "SimPlugins/PluginMessenger.h"

namespace sim {

PluginMessenger::PluginMessenger(PluginManager* thePluginManager)
    : pluginManager_(thePluginManager) {

    pluginDir_ = new G4UIdirectory("/ldmx/plugins/");
    pluginDir_->SetGuidance("Commands for controlling LDMX sim plugins");

    loadCmd_ = new G4UIcommand("/ldmx/plugins/load", this);
    loadCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);
    loadCmd_->SetGuidance("Load a sim plugin from a shared library.  A plugin of a specific type may only be loaded once.");
    G4UIparameter* pluginName = new G4UIparameter("pluginName", 's', false);
    pluginName->SetGuidance("Name of plugin to load.");
    loadCmd_->SetParameter(pluginName);
    G4UIparameter* libName = new G4UIparameter("libName", 's', true);
    libName->SetGuidance("Name of plugin library; if omitted will use the 'libSimPlugins.so' library.");
    loadCmd_->SetParameter(libName);

    destroyCmd_ = new G4UIcommand("/ldmx/plugins/destroy", this);
    destroyCmd_->SetGuidance("Destroy a loaded plugin by name.");
    destroyCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);
    pluginName = new G4UIparameter("pluginName", 's', false);
    pluginName->SetGuidance("Name of plugin to destroy.");
    destroyCmd_->SetParameter(pluginName);

    listCmd_ = new G4UIcommand("/ldmx/plugins/list", this);
    listCmd_->SetGuidance("List currently loaded plugins.");
    listCmd_->AvailableForStates(G4ApplicationState::G4State_Idle, G4ApplicationState::G4State_PreInit);
}

PluginMessenger::~PluginMessenger() {
    delete pluginDir_;
    delete loadCmd_;
    delete destroyCmd_;
    delete listCmd_;
}

void PluginMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {

    std::istringstream is((const char*) newValues);
    std::string pluginName, libName;

    is >> pluginName >> libName;

    if (libName.size() == 0 || libName == "") {
        libName = "libSimPlugins.so";
    }

    if (command == loadCmd_) {
        pluginManager_->create(pluginName, libName);
    } else if (command == destroyCmd_) {
        pluginManager_->destroy(pluginName);
    } else if (command == listCmd_) {
        pluginManager_->print(std::cout);
    }
}

}
