
#include "Biasing/EcalProcessFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4RunManager.hh"
#include "G4EventManager.hh"
#include "G4Step.hh"
#include "G4Track.hh" 

/*~~~~~~~~~~~~~*/
/*   SimApplication   */
/*~~~~~~~~~~~~~*/
#include "SimApplication/UserTrackInformation.h"
#include "SimApplication/UserEventInformation.h" 
#include "SimApplication/UserTrackInformation.h"

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

        // Get the particle type.
        G4String particleName = track->GetParticleDefinition()->GetParticleName();


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


            // Check if the electron will be exiting the target        
            if (auto volume{track->GetNextVolume()->GetName()}; volume.compareTo("hcal_PV") == 0) {
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
            trackInfo->tagPNGamma(); 
            eventInfo->decBremCandidateCount(); 
            eventInfo->setWeight(track->GetWeight());  
        }

    }
}

DECLARE_ACTION(ldmx, EcalProcessFilter)
