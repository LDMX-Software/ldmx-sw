/**
 * @file EcalENFilter.cxx
 * @brief Class defining a UserActionPlugin that biases Geant4 to only process 
 *        events which involve an electronuclear reaction in the target
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Biasing/EcalENFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4RunManager.hh"

namespace ldmx { 

    EcalENFilter::EcalENFilter(const std::string& name, Parameters& parameters) 
        : UserAction (name, parameters) {
        
        recoilEnergyThreshold_ = parameters.getParameter< double >("recoilThreshold"); 

    }

    EcalENFilter::~EcalENFilter() {}

    void EcalENFilter::stepping(const G4Step* step) { 

        // Only continue if we aren't aborted
        if (G4EventManager::GetEventManager()->GetConstCurrentEvent()->IsAborted()) return;

        // Only process tracks whose parent is the primary particle
        if (step->GetTrack()->GetParentID() != 0) return; 

        //track is the primary electron and event hasn't been aborted yet
        auto start{step->GetPreStepPoint()};
        auto end{step->GetPostStepPoint()};
        if ( (start->GetKineticEnergy() < recoilEnergyThreshold_ and end->GetKineticEnergy() > recoilEnergyThreshold_)
              or 
             (start->GetPhysicalVolume()->GetLogicalVolume()->GetRegion()->GetName().compareTo("CalorimeterRegion")==0 and
              end  ->GetPhysicalVolume()->GetLogicalVolume()->GetRegion()->GetName().compareTo("CalorimeterRegion")!=0)
           ) {

            //we just stepped below the threshold or left the calorimeter region
            
            // Get the electron secondries
            auto secondaries = step->GetSecondary();
            std::cout << "[ EcalBremFilter ] : Primary electron went below recoil energy threshold: " 
                << "KE: " << step->GetTrack()->GetKineticEnergy() << "MeV "
                << "N Secondaries: " << secondaries->size() << std::endl;
            /*
            */

            if (!secondaries or secondaries->size() == 0) {
                std::cout << "[ EcalBremFilter ] : "
                    << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
                    << " No secondaries at all. Aborting event..." << std::endl;
                /*
                */
                step->GetTrack()->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                return;
            }

            double enEnergy(0.);
            for (auto& secondary_track : *secondaries) {
                G4String processName = secondary_track->GetCreatorProcess()->GetProcessName();
                
                if (processName.contains("electronNuclear")) {
                    std::cout << "[ EcalENFilter ] : "
                        << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID() 
                        << " Found a secondary EN product with energy "
                        << secondary_track->GetKineticEnergy() << " MeV."
                        << std::endl;
                    /*
                    */
                    enEnergy += secondary_track->GetKineticEnergy(); 
                } //check for hard recoil
            }//loop over secondaries
    
            if (enEnergy < recoilEnergyThreshold_) { 
                std::cout << "[ EcalBremFilter ] : "
                    << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
                    << " Not enough energy went to EN secondaries. Aborting event..." << std::endl;
                /*
                */
                step->GetTrack()->SetTrackStatus(fKillTrackAndSecondaries);
                G4RunManager::GetRunManager()->AbortEvent();
                return;
            }
        }// electron stepped below recoil energy threshold or left calorimeter region
    }    

}

DECLARE_ACTION(ldmx, EcalENFilter) 
