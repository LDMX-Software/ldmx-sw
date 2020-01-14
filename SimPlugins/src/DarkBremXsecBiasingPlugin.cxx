/**
 * @file DarkBremXsecBiasingPlugin.cxx
 * @brief Run action plugin that biases the Geant4 Dark Brem xsec by a user
 *        specified value. 
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimPlugins/DarkBremXsecBiasingPlugin.h"

// Geant4
#include "G4Electron.hh"
#include "G4VEnergyLossProcess.hh"
#include "G4BiasingProcessInterface.hh"
#include "G4RunManager.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessTable.hh"

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

        if (dynamic_cast<G4BiasingProcessInterface*>(process)) {
            //resetting process to the real process (the one contained within the biasing wrapper)
            process = dynamic_cast<G4BiasingProcessInterface*>(process)->GetWrappedProcess();
        }

        G4eDarkBremsstrahlung *eDarkBrem = dynamic_cast<G4eDarkBremsstrahlung*>(process);
        if (eDarkBrem) {
            //process is Dark Brem process

            eDarkBrem->SetCrossSectionBiasingFactor(xsecBiasingFactor_);
            eDarkBrem->SetMethod(method_);
            eDarkBrem->SetMadGraphDataFile(madGraphDataFile_);

            if ( this->getVerboseLevel() > 1 ) {
    	        std::cout << "[ DarkBremXsecBiasingPlugin ]: " 
                    << "Dark Brem xsec has increased by a factor of " << xsecBiasingFactor_ << std::endl;
    	        std::cout << "[ DarkBremXsecBiasingPlugin ]: " 
                    << "Dark Brem simulation mode set to " << method_ << std::endl;
            }// verbose
        } //if process is Dark Brem
    }//loop through process list
}

void ldmx::DarkBremXsecBiasingPlugin::endEvent(const G4Event*) {

    //Re-activate the process at the end of each event. 
    //The process is deactivated each time it occurs, to limit it to one brem per event.
    G4bool active = true;
    G4String pname = "eDBrem";
    G4ProcessTable* ptable = G4ProcessTable::GetProcessTable();
    ptable->SetProcessActivation(pname,active);    
    if ( this->getVerboseLevel() > 1 ) {
        std::cout << "Reset the dark brem process.\n";
    }
}
