/**
 * @file TargetPhotonuclearBiasing.cxx
 * @brief User action plugin that biases Geant4 to only process events which
 *        involve a photonuclear reaction in the target.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Biasing/TargetProcessFilter.h"

/*~~~~~~~~~~~~~*/
/*   Biasing   */
/*~~~~~~~~~~~~~*/
#include "Biasing/TargetBremFilter.h"

namespace ldmx { 

    TargetProcessFilter::TargetProcessFilter(const std::string& name, Parameters& parameters) 
        : UserAction(name, parameters) {
        
        volumeName_ = parameters.getParameter< std::string >("volume");
        photonEnergyThreshold_ = parameters.getParameter< double >("photonThreshold"); 
        process_ = parameters.getParameter< std::string >("process");  
    }

    TargetProcessFilter::~TargetProcessFilter() {}

    G4ClassificationOfNewTrack TargetProcessFilter::ClassifyNewTrack(
            const G4Track* track, 
            const G4ClassificationOfNewTrack& currentTrackClass) {

        /*ldmx_log(debug) << "********************************" << std::endl; 
        ldmx_log(debug) << "*   Track pushed to the stack  *" << std::endl;
        ldmx_log(debug) << "********************************" << std::endl;*/

        // get the PDGID of the track.
        //G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

        // Get the particle type.
        G4String particleName = track->GetParticleDefinition()->GetParticleName();

        /*ldmx_log(debug) << "[ TargetBremFilter ]: " << "\n" 
                    << "\tParticle " << particleName << " ( PDG ID: " << pdgID << " ) : " << "\n"
                    << "\tTrack ID: " << track->GetTrackID() << "\n" 
                    << std::endl;*/

        if (track == currentTrack_) {
            currentTrack_ = nullptr; 
            //ldmx_log(debug) << "[ TargetBremFilter ]: Pushing track to waiting stack." << std::endl;
            return fWaiting; 
        }

        // Use current classification by default so values from other plugins are not overridden.
        G4ClassificationOfNewTrack classification = currentTrackClass;
        
        return classification;
    }

    void TargetProcessFilter::stepping(const G4Step* step) { 

        if (TargetBremFilter::getBremGammaList().empty() || reactionOccurred_) { 
            return;
        } 

        // Get the track associated with this step.
        G4Track* track = step->GetTrack();

        // Only process tracks whose parent is the primary particle
        if (track->GetParentID() != 1) return; 

        // get the PDGID of the track.
        G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

        // Make sure that the particle being processed is an electron.
        if (pdgID != 22) return; // Throw an exception

        // Get the volume the particle is in.
        G4VPhysicalVolume* volume = track->GetVolume();
        G4String volumeName = volume->GetName();

        // If the particle isn't in the target, don't continue with the processing.
        if (volumeName.compareTo(volumeName_) != 0) return;

        /*ldmx_log(debug) << "*******************************" << std::endl; 
        ldmx_log(debug) << "*   Step " << track->GetCurrentStepNumber() << std::endl;
        ldmx_log(debug) << "********************************" << std::endl;*/

        // Get the particle type.
        G4String particleName = track->GetParticleDefinition()->GetParticleName();

        // Get the kinetic energy of the particle.
        //double incidentParticleEnergy = step->GetPreStepPoint()->GetTotalEnergy();

        /*ldmx_log(debug) << "[ TargetProcessFilter ]: " << "\n" 
                    << "\tTotal energy of " << particleName      << " ( PDG ID: " << pdgID
                    << " ) : " << incidentParticleEnergy       << "\n"
                    << "\tTrack ID: " << track->GetTrackID()     << "\n" 
                    << "\tStep #: " << track->GetCurrentStepNumber() << "\n"
                    << "\tParticle currently in " << volumeName  << std::endl;*/
        
        // 
        std::vector<G4Track*> bremGammaList = TargetBremFilter::getBremGammaList();
        if (std::find(bremGammaList.begin(), bremGammaList.end(), track) == bremGammaList.end()) { 
            /*ldmx_log(debug) << "[ TargetProcessFilter ]: "
                      << "Brem list doesn't contain track." << std::endl;*/
            currentTrack_ = track; 
            track->SetTrackStatus(fSuspend);
            return;
        }

        // Get the particles daughters.
        const G4TrackVector* secondaries = step->GetSecondary();

        // If the brem photon doesn't undergo any reaction in the target, stop
        // processing the rest of the event. 
        if (secondaries->size() == 0) {

            /*ldmx_log(debug) << "[ TargetProcessFilter ]: "
                        << "Brem photon did not interact in the target. --> Postponing tracks."
                        << std::endl;*/
            

            if (bremGammaList.size() == 1) { 
                track->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                currentTrack_ = nullptr;
                return;
            } else {
                currentTrack_ = track; 
                track->SetTrackStatus(fSuspend);
                TargetBremFilter::removeBremFromList(track);
                return;
            }
        } else { 
       
            G4String processName = secondaries->at(0)->GetCreatorProcess()->GetProcessName(); 
            
            /*ldmx_log(debug) << "[ TargetProcessFilter ]: "
                      << "Brem photon produced " << secondaries->size() 
                      << " particle via " << processName << " process." 
                      << std::endl;*/

            // Only record the process that is being biased
            if (!processName.contains(process_)) {

                /*ldmx_log(debug) << "[ TargetProcessFilter ]: "
                          << "Process was not " << BiasingMessenger::getProcess() << "--> Killing all tracks!" 
                          << std::endl;*/
                
                if (bremGammaList.size() == 1) { 
                    track->SetTrackStatus(fKillTrackAndSecondaries);
                    G4RunManager::GetRunManager()->AbortEvent();
                    currentTrack_ = nullptr;
                    return;
                } else { 
                    currentTrack_ = track; 
                    track->SetTrackStatus(fSuspend);
                    TargetBremFilter::removeBremFromList(track);
                    return;
                }
            }

            ldmx_log(debug) << "[ TargetProcessFilter ]: "
                      << "Brem photon produced " << secondaries->size() 
                      << " particle via " << processName << " process.";
            TargetBremFilter::removeBremFromList(track);
            //BiasingMessenger::setEventWeight(track->GetWeight());
            reactionOccurred_ = true;
        }
    }    

    void TargetProcessFilter::EndOfEventAction(const G4Event*) {
        reactionOccurred_ = false; 
    }
}

