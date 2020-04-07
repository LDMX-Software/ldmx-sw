/*
 * @file EcalProcessFilter.cxx
 * @brief User action plugin that biases Geant4 to only process events which
 *        involve a photonuclear reaction in the ECal.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Biasing/EcalProcessFilter.h"

namespace ldmx { 

    EcalProcessFilter::EcalProcessFilter(const std::string& name, Parameters& parameters) 
        : UserAction(name, parameters) {

        boundVolumes_ = parameters.getParameter< std::vector < std::string > >("boundVolumes"); 
        auto volume{parameters.getParameter< std::string >("volume")};
        addVolume(volume); 
        process_ = parameters.getParameter< std::string >("process");  
    }

    EcalProcessFilter::~EcalProcessFilter() {
    }

    G4ClassificationOfNewTrack EcalProcessFilter::ClassifyNewTrack(
            const G4Track* track, 
            const G4ClassificationOfNewTrack& currentTrackClass) {

        /*std::cout << "********************************" << std::endl;*/ 
        /*std::cout << "*   Track pushed to the stack  *" << std::endl;*/
        /*std::cout << "********************************" << std::endl;*/

        // get the PDGID of the track.
        //G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

        // Get the particle type.
        G4String particleName = track->GetParticleDefinition()->GetParticleName();

        /*std::cout << "[ TargetBremFilter ]: " << "\n" 
                    << "\tParticle " << particleName << " ( PDG ID: " << pdgID << " ) : " << "\n"
                    << "\tTrack ID: " << track->GetTrackID() << "\n" 
                    << std::endl;*/

        if (track == currentTrack_) {
            currentTrack_ = nullptr; 
            return fWaiting; 
        }

        // Use current classification by default so values from other plugins are not overridden.
        G4ClassificationOfNewTrack classification = currentTrackClass;
        
        return classification;
    }

    void EcalProcessFilter::stepping(const G4Step* step) { 

        if (TargetBremFilter::getBremGammaList().empty()) { 
            return;
        } 
        
        // Get the track associated with this step.
        G4Track* track = step->GetTrack();

        // Only process tracks whose parent is the primary particle
        if (track->GetParentID() != 1) return; 

        // get the PDGID of the track.
        G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();

        // Make sure that the particle being processed is an electron.
        // TODO: At some point all particle types should be allowed.
        if (pdgID != 22) return; 

        // Get the volume the particle is in.
        G4VPhysicalVolume* volume = track->GetVolume();
        G4String volumeName = volume->GetName();

        //std::cout << "*******************************" << std::endl;
        //std::cout << "*   Step " << track->GetCurrentStepNumber() << std::endl;
        //std::cout << "********************************" << std::endl;
        
        // Get the particle type.
        G4String particleName = track->GetParticleDefinition()->GetParticleName();
        
        // Get the kinetic energy of the particle.
        //double incidentParticleEnergy = step->GetPreStepPoint()->GetTotalEnergy();

        //std::cout << "[ EcalProcessFilter ]:\n" 
        //            << "\tTotal energy of " << particleName  << ": " << incidentParticleEnergy << " MeV \n"
        //            << "\tPDG ID: " << pdgID << "\n"
        //            << "\tTrack ID: " << track->GetTrackID() << "\n" 
        //            << "\tStep #: " << track->GetCurrentStepNumber() << "\n"
        //            << "\tParent ID: " << track->GetParentID() << "\n"
        //            << "\tParticle currently in " << volumeName  << std::endl;

        // If the particle isn't in the specified volume, stop processing the 
        // event.
        std::vector<G4Track*> bremGammaList = TargetBremFilter::getBremGammaList();
        if (std::find(std::begin(volumes_), std::end(volumes_), volumeName) == std::end(volumes_)) {

            //std::cout << "[ EcalProcessFilter ]: "
            //            << "Brem is in " << volumeName  << std::endl;

            // If secondaries were produced outside of the volume of interest, 
            // and there aren't additional brems to process, abort the 
            // event.  Otherwise, suspend the track and move on to the next 
            // brem.
            if (step->GetSecondary()->size() != 0 
                    && (std::find(bremGammaList.begin(), bremGammaList.end(), track) != bremGammaList.end())) { 
                
                //std::cout << "[ EcalProcessFilter ]: "
                //            << "Reaction occured outside volume of intereset --> Aborting event." 
                //            << std::endl;

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
            //std::cout << "[ EcalProcessFilter ]: Returning." << std::endl;
            return;
        }

        // The list of brems will only contain a given track/particle if it 
        // originates from the target.  If the gamma originates elsewhere, 
        // suspend it and move on to the next gamma.
        // TODO: At some point, this needs to be modified to include brems 
        // from downstream of the target.
        if (std::find(bremGammaList.begin(), bremGammaList.end(), track) == bremGammaList.end()) { 
            
            /*std::cout << "[ EcalProcessFilter ]: "
                        << "Brem list doesn't contain track." << std::endl;*/
            
            currentTrack_ = track; 
            track->SetTrackStatus(fSuspend);
            return;
        }
 
        // Get the particles daughters.
        const G4TrackVector* secondaries = step->GetSecondary();
        //std::cout << "[ EcalProcessFilter ]: Secondaries: " << secondaries->size() << std::endl;


        // If the particle doesn't interact, then move on to the next step.
        if (secondaries->size() == 0) { 
            
            //std::cout << "[ EcalProcessFilter ]: "
            //            << "Brem photon did not interact --> Continue propogating track."
            //            << std::endl;   
        
            // If the particle is exiting the bounding volume, kill it.
            if (!boundVolumes_.empty() && step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) {
                if (std::find(std::begin(boundVolumes_), std::end(boundVolumes_), volumeName) != std::end(boundVolumes_)) {

                    //std::cout << "[ EcalProcessFilter ]: "
                    //            << "Brem photon is exiting the volume --> particle will be killed or suspended."
                    //            << std::endl;  
                    
                    if (bremGammaList.size() == 1) { 
                        track->SetTrackStatus(fKillTrackAndSecondaries);
                        G4RunManager::GetRunManager()->AbortEvent();
                        currentTrack_ = nullptr;
                        //std::cout << "[ EcalProcessFilter ]: " 
                        //            << " Brem list is empty --> Killing all tracks!"
                        //            << std::endl;
                        return;
                    } else { 
                        currentTrack_ = track; 
                        track->SetTrackStatus(fSuspend);
                        TargetBremFilter::removeBremFromList(track);
                        //std::cout << "[ EcalProcessFilter ]: " 
                        //            << " Other tracks still need to be processed --> Suspending track!"
                        //            << std::endl;
                        return;
                    }
                }
            }

        } else {

            // If the brem gamma interacts and produces secondaries, get the 
            // process used to create them. 
            G4String processName = secondaries->at(0)->GetCreatorProcess()->GetProcessName(); 
            
            //std::cout << "[ EcalProcessFilter ]: "
            //            << "Brem photon produced " << secondaries->size() 
            //            << " particle via " << processName << " process." 
            //            << std::endl;

            // Only record the process that is being biased
            if (!processName.contains(process_)) {

                //std::cout << "[ EcalProcessFilter ]: "
                //            << "Process was not " << process_ 
                //            << std::endl;
                
                if (bremGammaList.size() == 1) { 
                    track->SetTrackStatus(fKillTrackAndSecondaries);
                    G4RunManager::GetRunManager()->AbortEvent();
                    currentTrack_ = nullptr;
                    //std::cout << "[ EcalProcessFilter ]: " 
                    //          << " Brem list is empty --> Killing all tracks!"
                    //          << std::endl;
                    return;
                } else { 
                    currentTrack_ = track; 
                    track->SetTrackStatus(fSuspend);
                    TargetBremFilter::removeBremFromList(track);
                    //std::cout << "[ EcalProcessFilter ]: " 
                    //            << " Other tracks still need to be processed --> Suspending track!"
                    //            << std::endl;
                    return;
                }
            }
            
            ldmx_log(debug) << "[ EcalProcessFilter ]: "
                      << "Brem photon produced " << secondaries->size() 
                      << " particle via " << processName << " process." ;
            TargetBremFilter::removeBremFromList(track);
            //BiasingMessenger::setEventWeight(track->GetWeight());
            photonGammaID_ = track->GetTrackID(); 
        }

    }

    void EcalProcessFilter::PostUserTrackingAction(const G4Track* track) { 
       
        if (track->GetParentID() == photonGammaID_) { 
            UserTrackInformation* userInfo 
              = dynamic_cast<UserTrackInformation*>(track->GetUserInformation());
            userInfo->setSaveFlag(true); 
            // get the PDGID of the track.
            //G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();
            G4ThreeVector pvec = track->GetMomentum();
            /*std::cout << "[ EcalProcessFilter ]:\n" 
                        << "\tPDG ID: " << pdgID << "\n"
                        << "\tTrack ID: " << track->GetTrackID() << "\n" 
                        << "\tStep #: " << track->GetCurrentStepNumber() << "\n"
                        << "\tParent ID: " << track->GetParentID() << "\n"
                        << "\t p: [ " << pvec[0] << ", " << pvec[1] << ", " << pvec[2] << " ]" << std::endl;*/
        }
    }

    void EcalProcessFilter::addVolume(std::string volume) { 
        
        if (volume.compare("ecal") == 0) { 
            for (G4VPhysicalVolume* physVolume : *G4PhysicalVolumeStore::GetInstance()) {
                G4String physVolumeName = physVolume->GetName();
                if ((physVolumeName.contains("W") || physVolumeName.contains("Si")) 
                        && physVolumeName.contains("phys")) {
                    volumes_.push_back(physVolumeName);
                }
            }
        } else { 
            volumes_.push_back(volume);
        } 
    }        
    
    void EcalProcessFilter::addBoundingVolume(std::string volume) { 
        boundVolumes_.push_back(volume);
    }        
}

DECLARE_ACTION(ldmx, EcalProcessFilter)
