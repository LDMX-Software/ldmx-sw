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

void ldmx::DarkBremXsecBiasingPlugin::endEvent(const G4Event*) {

    //Re-activate the process at the end of each event. 
    //The process is deactivated each time it occurs, to limit it to one brem per event.
    G4bool active = true;
    G4String pname = "biasWrapper(eDBrem)";
    G4ProcessTable* ptable = G4ProcessTable::GetProcessTable();
    ptable->SetProcessActivation(pname,active);    
    if ( this->getVerboseLevel() > 1 ) {
        std::cout << "[ DarkBremXsecBiasingPlugin ] : "
                  << "Reset the dark brem process." << std::endl;
    }
}
