/**
 * @file PhotonuclearXsecBiasingPlugin.h
 * @brief Run action plugin that biases the Geant4 photonuclear xsec by a user
 *        specified value. 
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#include "SimPlugins/PhotonuclearXsecBiasingPlugin.h"

extern "C" ldmx::PhotonuclearXsecBiasingPlugin* createPhotonuclearXsecBiasingPlugin() {
    return new ldmx::PhotonuclearXsecBiasingPlugin;
}

extern "C" void destroyPhotonuclearXsecBiasingPlugin(ldmx::PhotonuclearXsecBiasingPlugin* object) {
    delete object;
}


ldmx::PhotonuclearXsecBiasingPlugin::PhotonuclearXsecBiasingPlugin() {
}

ldmx::PhotonuclearXsecBiasingPlugin::~PhotonuclearXsecBiasingPlugin() { 
    delete messenger_;
}

void ldmx::PhotonuclearXsecBiasingPlugin::beginRun(const G4Run*) { 
    
    // Get the process manager associated with reactions involving photons.
    G4ProcessManager* pm = G4Gamma::GammaDefinition()->GetProcessManager();

    // Get the list of available processes.
    G4ProcessVector* processes = pm->GetProcessList();

    // Bias the photonuclear process by the user specified factor.
    for (int processIndex = 0; processIndex < processes->entries(); processIndex++) {
        
        G4VProcess* process = (*processes)[processIndex];
        
        if (process->GetProcessName().compareTo("photonNuclear") == 0) {
        
            ((G4HadronicProcess*) process)->BiasCrossSectionByFactor(xsecBiasingFactor_);
            
            std::cout << "[ PhotonuclearXsecBiasingPlugin ]: "
                      << "Photonuclear xsec has increased by a factor of " 
                      << xsecBiasingFactor_ << std::endl;
        }
    }
}
