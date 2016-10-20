#include "SimPlugins/PluginMessenger.h"

namespace sim {

PluginMessenger::PluginMessenger(PluginManager* thePluginManager)
    : pluginManager(thePluginManager) {

    pluginDir = new G4UIdirectory("/ldmx/plugins/");
    pluginDir->SetGuidance("Commands for controlling LDMX sim plugins");

    loadCmd = new G4UIcommand("/ldmx/plugins/load", this);
    loadCmd->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);
    loadCmd->SetGuidance("Load a sim plugin from a shared library.  A plugin of a specific type may only be loaded once.");
    G4UIparameter* pluginName = new G4UIparameter("pluginName", 's', false);
    pluginName->SetGuidance("Name of plugin to load.");
    loadCmd->SetParameter(pluginName);
    G4UIparameter* libName = new G4UIparameter("libName", 's', true);
    libName->SetGuidance("Name of plugin library; if omitted will use the 'libSimPlugins.so' library.");
    loadCmd->SetParameter(libName);

    destroyCmd = new G4UIcommand("/ldmx/plugins/destroy", this);
    destroyCmd->SetGuidance("Destroy a loaded plugin by name.");
    destroyCmd->AvailableForStates(G4ApplicationState::G4State_Idle);
    pluginName = new G4UIparameter("pluginName", 's', false);
    pluginName->SetGuidance("Name of plugin to destroy.");
    destroyCmd->SetParameter(pluginName);

    listCmd = new G4UIcommand("/ldmx/plugins/list", this);
    listCmd->SetGuidance("List currently loaded plugins.");
    listCmd->AvailableForStates(G4ApplicationState::G4State_Idle, G4ApplicationState::G4State_PreInit);
}

PluginMessenger::~PluginMessenger() {
    delete pluginDir;
    delete loadCmd;
    delete destroyCmd;
    delete listCmd;
}

void PluginMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {

    std::istringstream is((const char*) newValues);
    std::string pluginName, libName;

    is >> pluginName >> libName;

    if (libName.size() == 0 || libName == "") {
        libName = "libSimPlugins.so";
    }

    if (command == loadCmd) {
        pluginManager->create(pluginName, libName);
    } else if (command == destroyCmd) {
        pluginManager->destroy(pluginName);
    } else if (command == listCmd) {
        pluginManager->print(std::cout);
    }
}

}
