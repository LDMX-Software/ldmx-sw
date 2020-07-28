/*
 * @file DarkBremFilter.cxx
 * @class DarkBremFilter
 * @brief Class defining a UserActionPlugin that allows a user to filter out 
 *        events that don't result in a dark brem within a given volume
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Biasing/DarkBremFilter.h"

#include "SimCore/G4APrime.h" //checking if particles match A'
#include "SimCore/UserTrackInformation.h" //make sure A' is saved
#include "SimCore/UserEventInformation.h"

#include "G4LogicalVolumeStore.hh" //for the store

namespace ldmx { 

    DarkBremFilter::DarkBremFilter(const std::string& name, Parameters& parameters)
        : UserAction(name, parameters) {

        std::string desiredVolume = parameters.getParameter< std::string >("volume");
        nGensFromPrimary_ = parameters.getParameter< int >("nGensFromPrimary");
        minApEnergy_ = parameters.getParameter< double >("minApEnergy");
 
        //TODO check if this needs to be updated when v12 geo updates are merged in
        for (G4LogicalVolume* volume : *G4LogicalVolumeStore::GetInstance()) {
            G4String volumeName = volume->GetName();
            if ((desiredVolume.compare("ecal") == 0) ) {
                //looking for ecal volumes
                if ( volumeName.contains("volume") and
                    (   volumeName.contains("Si") 
                     or volumeName.contains("W") 
                     or volumeName.contains("CFMix") 
                     or volumeName.contains("PCB")
                    ) 
                ) { volumes_.push_back( volume ); }

            } else if (volumeName.contains(desiredVolume)) {
                volumes_.push_back( volume );
            }
        }

        if ( G4RunManager::GetRunManager()->GetVerboseLevel() > 0 ) {
            std::cout << "[ DarkBremFilter ]: "
                << "Looking for A' in: ";
            for ( auto const& volume : volumes_ ) std::cout << volume->GetName() << ", ";
            std::cout << std::endl;
        }
    }

    void DarkBremFilter::BeginOfEventAction(const G4Event*) {
        std::cout << "[ DarkBremFilter ]: "
            << "(" << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID() << ") "
            << "Beginning event."
            << std::endl;
        currentGen_ = 0;
        foundAp_    = false;
        return;
    }

    G4ClassificationOfNewTrack DarkBremFilter::ClassifyNewTrack(const G4Track* aTrack, const G4ClassificationOfNewTrack& ) {

        if ( aTrack->GetParticleDefinition() == G4APrime::APrime() ) {
            //there is an A'! Yay!
            //  we need to check that it originated in the desired volume
            //  keep A' in the current generation so that we can have it be processed
            //  before the abort event check
            if ( G4RunManager::GetRunManager()->GetVerboseLevel() > 1 ) {
                std::cout << "[ DarkBremFilter ]: "
                          << "Found A', still need to check if it originated in requested volume." 
                          << std::endl;
            }
            if ( foundAp_ ) {
                std::cout << "[ DarkBremFilter ]: "
                          << "Found more than one A' during filtering."
                          << std::endl;
            }
            foundAp_ = true;
            return fUrgent; 
        }

        return fWaiting;
    }

    void DarkBremFilter::NewStage() {

        /**
         * called when urgent stack is empty 
         * since we are putting everything on waiting stack, 
         * this is only called when a generation has been simulated 
         * and we are starting the next one.
         */

        /* Check that generational stacking is working
        std::cout << "[ DarkBremFilter ]: "
            << "Closing up generation " << currentGen_ << " and starting a new one."
            << std::endl;
        */

        //increment current generation
        currentGen_++;

        if ( currentGen_ > nGensFromPrimary_ ) {
            //we finished the number of generations that are allowed to produce A'
            //  check if A' was produced
            if ( not foundAp_ ) {
                //A' wasn't produced, abort event
                if ( G4RunManager::GetRunManager()->GetVerboseLevel() > 1 ) {
                    std::cout << "[ DarkBremFilter ]: "
                        << "A' wasn't produced, aborting event." 
                        << std::endl;
                }
                G4RunManager::GetRunManager()->AbortEvent();
            }
        }
        
        return;
    }

    void DarkBremFilter::PostUserTrackingAction(const G4Track* track) {

        /* Check that generational stacking is working
        std::cout << "[ DarkBremFilter ]:"
            << track->GetTrackID() << " " << track->GetParticleDefinition()->GetPDGEncoding()
            << std::endl;
        */

        //add generation to UserTrackInformation
        UserTrackInformation* userInfo 
              = dynamic_cast<UserTrackInformation*>(track->GetUserInformation());

        userInfo->setGeneration( currentGen_ );
        
        if ( track->GetParticleDefinition() == G4APrime::APrime() ) {
            //check if A' was made in the desired volume and has the minimum energy
            auto event{G4EventManager::GetEventManager()};
            if ( track->GetTotalEnergy() < minApEnergy_ or not inDesiredVolume(track) ) {
                //abort event if A' wasn't in correct volume or didn't have enough energy
                //if ( G4RunManager::GetRunManager()->GetVerboseLevel() > 1 ) {
                    std::cout << "[ DarkBremFilter ]: "
                        << "(" << event->GetConstCurrentEvent()->GetEventID() << ") "
                        << "A' wasn't produced inside of requested volume or was below requested energy, aborting event." 
                        << " A' was produced in '" << track->GetLogicalVolumeAtVertex()->GetName() << "' and had energy "
                        << track->GetTotalEnergy() << " MeV."
                        << std::endl;
                //}
                G4RunManager::GetRunManager()->AbortEvent();
            } else {
                //make sure A' is persisted into output file
                userInfo->setSaveFlag(true); 
                //we pushed A' through a generation early to make sure it gets
                // processed before the final check, so its generation is later
                userInfo->setGeneration( currentGen_+1 );
    
                if ( !event->GetUserInformation() ) {
                    event->SetUserInformation(new UserEventInformation);
                }
                static_cast<UserEventInformation*>(event->GetUserInformation())->setWeight( track->GetWeight() );
    
                if ( track->GetWeight() < 1e-13 ) {
                    std::cout << "[ DarkBremFilter ]: "
                        << "Event " << event->GetConstCurrentEvent()->GetEventID()
                        << " with weight less than 1./Biasing Factor: " << track->GetWeight()
                        << ". A' was produced in '" << track->GetLogicalVolumeAtVertex()->GetName() << "' and had energy "
                        << track->GetTotalEnergy() << " MeV."
                        << std::endl;
                    userInfo->Print();
                    event->GetUserInformation()->Print();
                }
            } //A' was made in desired volume and has the minimum energy
        }//track is A'

        return;
    }

    bool DarkBremFilter::inDesiredVolume(const G4Track* track) const {

        /**
         * Comparing the pointers to logical volumes isn't very robust.
         * TODO find a better way to do this
         */

        auto inVol = track->GetLogicalVolumeAtVertex();
        for ( auto const& volume : volumes_ ) {
            if ( inVol == volume ) return true;
        }

        return false;
    }
}

DECLARE_ACTION(ldmx, DarkBremFilter) 
