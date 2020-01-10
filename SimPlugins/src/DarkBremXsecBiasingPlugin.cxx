/**
 * @file DarkBremXsecBiasingPlugin.h
 * @brief Run action plugin that biases the Geant4 Dark Brem xsec by a user
 *        specified value. 
 * @author Michael Revering
 *         University of Minnesota
 */

#include "SimPlugins/DarkBremXsecBiasingPlugin.h"

SIM_PLUGIN(ldmx, DarkBremXsecBiasingPlugin)

ldmx::DarkBremXsecBiasingPlugin::DarkBremXsecBiasingPlugin() {
}

ldmx::DarkBremXsecBiasingPlugin::~DarkBremXsecBiasingPlugin() {
    delete messenger_;
}

void ldmx::DarkBremXsecBiasingPlugin::beginRun(const G4Run*) {

    // Get the process manager associated with reactions involving photons.
    G4ProcessManager* pm = G4Electron::ElectronDefinition()->GetProcessManager();

    // Get the list of available processes.
    G4ProcessVector* processes = pm->GetProcessList();

    // Bias the Dark Brem process by the user specified factor.
    for (int processIndex = 0; processIndex < processes->entries(); processIndex++) {

        G4VProcess* process = (*processes)[processIndex];

        if (process->GetProcessName().compareTo("eDBrem") == 0) {

            ((G4VEnergyLossProcess*)process)->SetCrossSectionBiasingFactor(xsecBiasingFactor_);
            ((G4eDarkBremsstrahlung*)process)->SetMethod(mode_);

	    std::cout << "[ DarkBremXsecBiasingPlugin ]: " << "Dark Brem xsec has increased by a factor of " << xsecBiasingFactor_ << std::endl;
	    std::cout << "[ DarkBremXsecBiasingPlugin ]: " << "Dark Brem simulation mode set to " << mode_ << std::endl;
        }
    }
}

void ldmx::DarkBremXsecBiasingPlugin::endEvent(const G4Event*) {

    //Re-activate the process at the end of each event. The process is deactivated each time it occurs, to limit it to one brem per event.
    G4bool active = true;
    G4String pname = "biasWrapper(eDBrem)";
    G4ProcessTable* ptable = G4ProcessTable::GetProcessTable();
    ptable->SetProcessActivation(pname,active);    
    std::cout << "Reset the dark brem process.\n";

}
