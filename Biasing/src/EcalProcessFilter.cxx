/*
 * @file EcalProcessFilter.cxx
 * @brief User action plugin that biases Geant4 to only process events which
 *        involve a photonuclear reaction in the ECal.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Biasing/EcalProcessFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4RunManager.hh"
#include "G4EventManager.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserTrackInformation.h"
#include "SimApplication/UserEventInformation.h" 
#include "SimCore/UserTrackInformation.h"

namespace ldmx { 

    EcalProcessFilter::EcalProcessFilter(const std::string& name, Parameters& parameters) 
        : UserAction(name, parameters) {
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

        // Get the track associated with this step.
        auto track{step->GetTrack()};

        // Get the track info and check if this track is a brem candidate
        auto trackInfo{static_cast< UserTrackInformation* >(track->GetUserInformation())};
        if ((trackInfo != nullptr) && !trackInfo->isBremCandidate()) return;  
       
        // Get the event info to keep track of the number of brem candidates
        auto eventInfo{
            static_cast< UserEventInformation* >(
                    G4EventManager::GetEventManager()->GetUserInformation())};
        if (eventInfo == nullptr) {
            // thrown an exception
        }

        // Get the particles daughters.
        auto secondaries{step->GetSecondary()};

        // Get the region the particle is currently in.  Continue processing
        // the particle only if it's in the calorimeter region. 
        if (auto region{track->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()};
                region.compareTo("CalorimeterRegion") != 0) { 

            // If secondaries were produced outside of the volume of interest, 
            // and there aren't additional brems to process, abort the 
            // event.  Otherwise, suspend the track and move on to the next 
            // brem.
            if (secondaries->size() != 0) {
                    
                if (eventInfo->bremCandidateCount() == 1) {
                    track->SetTrackStatus(fKillTrackAndSecondaries);
                    G4RunManager::GetRunManager()->AbortEvent();
                    currentTrack_ = nullptr;
                } else { 
                    currentTrack_ = track; 
                    track->SetTrackStatus(fSuspend);
                    eventInfo->decBremCandidateCount();
                    trackInfo->tagBremCandidate(false);   
                }
            }
            return;
        }

        // If the particle doesn't interact, then move on to the next step.
        if (secondaries->size() == 0) { 
            
            //std::cout << "[ EcalProcessFilter ]: "
            //            << "Brem photon did not interact --> Continue propogating track."
            //            << std::endl;   
        
            // If the particle is exiting the bounding volume, kill it.
            /*if (!boundVolumes_.empty() && step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) {
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
            }*/
            return; 
        } else {

            // If the brem gamma interacts and produces secondaries, get the 
            // process used to create them. 
            auto processName{secondaries->at(0)->GetCreatorProcess()->GetProcessName()}; 
            
            // Only record the process that is being biased
            if (!processName.contains(process_)) {

                if (eventInfo->bremCandidateCount() == 1) {
                    track->SetTrackStatus(fKillTrackAndSecondaries);
                    G4RunManager::GetRunManager()->AbortEvent();
                    currentTrack_ = nullptr;
                } else { 
                    currentTrack_ = track; 
                    track->SetTrackStatus(fSuspend);
                    eventInfo->decBremCandidateCount();  
                    trackInfo->tagBremCandidate(false);   
                }
                return; 
            }
            
            std::cout << "[ EcalProcessFilter ]: "
                      << "Brem photon produced " << secondaries->size() 
                      << " particle via " << processName << " process." 
                      << std::endl;
            trackInfo->tagBremCandidate(false);   
            eventInfo->decBremCandidateCount();  
            //BiasingMessenger::setEventWeight(track->GetWeight());
        }

    }

    /*
    void EcalProcessFilter::PostUserTrackingAction(const G4Track* track) { 
       
        if (track->GetParentID() == photonGammaID_) { 
            UserTrackInformation* userInfo 
              = dynamic_cast<UserTrackInformation*>(track->GetUserInformation());
            userInfo->setSaveFlag(true); 
            // get the PDGID of the track.
            //G4int pdgID = track->GetParticleDefinition()->GetPDGEncoding();
            G4ThreeVector pvec = track->GetMomentum();
            std::cout << "[ EcalProcessFilter ]:\n" 
                        << "\tPDG ID: " << pdgID << "\n"
                        << "\tTrack ID: " << track->GetTrackID() << "\n" 
                        << "\tStep #: " << track->GetCurrentStepNumber() << "\n"
                        << "\tParent ID: " << track->GetParentID() << "\n"
                        << "\t p: [ " << pvec[0] << ", " << pvec[1] << ", " << pvec[2] << " ]" << std::endl;
        }
    }

    */    
}

DECLARE_ACTION(ldmx, EcalProcessFilter)
