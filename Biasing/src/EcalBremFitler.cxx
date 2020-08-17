
#include "Biasing/EcalBremFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4RunManager.hh"
#include "G4EventManager.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserTrackInformation.h"
#include "SimCore/UserEventInformation.h" 

namespace ldmx { 

    EcalBremFilter::EcalBremFilter(const std::string& name, Parameters& parameters) 
        : UserAction(name, parameters) { 
        
        bremEnergyThreshold_     = parameters.getParameter< double >("brem_min_energy_threshold"); 
    }

    EcalBremFilter::~EcalBremFilter() {
    }

    void EcalBremFilter::stepping(const G4Step* step) { 

        // Only process the primary electron track
        if (step->GetTrack()->GetParentID() != 0) return;

        if (G4EventManager::GetEventManager()->GetConstCurrentEvent()->IsAborted()) return;

        //track is the primary electron and event hasn't been aborted yet
        auto start{step->GetPreStepPoint()};
        auto end{step->GetPostStepPoint()};
        if ( (start->GetKineticEnergy() < bremEnergyThreshold_ and end->GetKineticEnergy() > bremEnergyThreshold_)
              or 
             (start->GetPhysicalVolume()->GetLogicalVolume()->GetRegion()->GetName().compareTo("CalorimeterRegion")==0 and
              end  ->GetPhysicalVolume()->GetLogicalVolume()->GetRegion()->GetName().compareTo("CalorimeterRegion")!=0)
           ) {

            //we just stepped below the threshold or left the calorimeter region
            
            // Get the electron secondries
            auto secondaries = step->GetSecondary();
            /*
            std::cout << "[ EcalBremFilter ] : Primary electron went below brem energy threshold: " 
                << "KE: " << step->GetTrack()->GetKineticEnergy() << "MeV "
                << "N Secondaries: " << secondaries->size() << std::endl;
            */

            if (!secondaries or secondaries->size() == 0) {
                /*
                std::cout << "[ EcalBremFilter ] : "
                    << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
                    << " No secondaries at all. Aborting event..." << std::endl;
                */
                step->GetTrack()->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                return;
            }

            bool hasBremCandidate = false; 
            for (auto& secondary_track : *secondaries) {
                G4String processName = secondary_track->GetCreatorProcess()->GetProcessName();
                
                if (processName.compareTo("eBrem") == 0 
                        && secondary_track->GetKineticEnergy() > bremEnergyThreshold_) {
    
                    /*
                    std::cout << "[ EcalBremFilter ] : "
                        << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID() 
                        << " Found a secondary hard brem! " 
                        << secondary_track->GetTrackID()
                        << std::endl;
                    */
                   
                    if (secondary_track->GetUserInformation() == nullptr) {
                        secondary_track->SetUserInformation(new UserTrackInformation()); 
                    }
                    auto trackInfo{static_cast< UserTrackInformation* >(secondary_track->GetUserInformation())};
                    trackInfo->tagBremCandidate(); 
                    trackInfo->setVertexVolume(secondary_track->GetVolume()->GetName()); 
    
                    auto event{G4EventManager::GetEventManager()}; 
                    if (event->GetUserInformation() == nullptr) { 
                        event->SetUserInformation(new UserEventInformation()); 
                    }
                    static_cast< UserEventInformation* >(event->GetUserInformation())->incBremCandidateCount(); 
    
                    hasBremCandidate = true;
                } //check for hard brem
            }//loop over secondaries
    
            if (!hasBremCandidate) { 
                /*
                std::cout << "[ EcalBremFilter ] : "
                    << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
                    << " No hard-brem secondaries. Aborting event..." << std::endl;
                */
                step->GetTrack()->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                return;
            }
        }// electron stepped below brem energy threshold or left calorimeter region
    }
}

DECLARE_ACTION(ldmx, EcalBremFilter) 
