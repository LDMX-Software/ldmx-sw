
#include "Biasing/PartialEnergySorter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4RunManager.hh" 
#include "G4EventManager.hh" 
#include "G4Step.hh"

namespace ldmx {

    PartialEnergySorter::PartialEnergySorter(const std::string& name, Parameters& parameters) 
        : UserAction(name, parameters) {
            threshold_ = parameters.getParameter< double >("threshold"); 
    }

    PartialEnergySorter::~PartialEnergySorter() { } 

    void PartialEnergySorter::BeginOfEventAction(const G4Event*) {
        /** debug printout
        std::cout << "[ PartialEnergySorter ] : "
            << "Starting a new event."
            << std::endl;
         */
        num_particles_above_threshold_ = 0;
    }

    G4ClassificationOfNewTrack PartialEnergySorter::ClassifyNewTrack(const G4Track* aTrack,
            const G4ClassificationOfNewTrack& currentTrackClassification) {

        /** debug printout
        std::cout << "[ PartialEnergySorter ] : Classifying track "
            << aTrack->GetTrackID() << " with energy "
            << aTrack->GetKineticEnergy() << " MeV."
            << std::endl;
         */

        if ( aTrack->GetKineticEnergy() > threshold_ ) {
            /** debug printout
            std::cout << "[ PartialEnergySorter ] : Classifying track "
                << aTrack->GetTrackID() << " with energy "
                << aTrack->GetKineticEnergy() << " MeV."
                << std::endl;
             */
            num_particles_above_threshold_++;
            return fUrgent;
        }

        /*
         * Track has kinetic energy less than or equal to
         * the threshold, so we put it on the waiting stack
         * if there are still particles above threshold to be processed.
         */
        return num_particles_above_threshold_ > 0 ? fWaiting : currentTrackClassification;
    }

    void PartialEnergySorter::stepping(const G4Step* step) {

        if ( num_particles_above_threshold_ == 0 ) return;

        auto pre_energy{step->GetPreStepPoint()->GetKineticEnergy()};
        auto post_energy{step->GetPostStepPoint()->GetKineticEnergy()};

        if ( pre_energy >= threshold_ and post_energy <= threshold_ ) {
            /** debug printout
            std::cout << "[ PartialEnergySorter ] : Stepping track "
                << step->GetTrack()->GetTrackID() << " going from "
                << pre_energy  << " MeV to "
                << post_energy << " MeV."
                << std::endl;
             */
            num_particles_above_threshold_--;
            step->GetTrack()->SetTrackStatus(fSuspend);
        }
    } 

} // ldmx

DECLARE_ACTION(ldmx, PartialEnergySorter)
