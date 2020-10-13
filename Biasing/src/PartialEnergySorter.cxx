
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

    G4ClassificationOfNewTrack PartialEnergySorter::ClassifyNewTrack(const G4Track* aTrack,
            const G4ClassificationOfNewTrack& currentTrackClassification) final override {

        if ( aTrack->GetTotalEnergy() > threshold_ ) {
            num_particles_above_threshold_++;
            return fUrgent;
        }

        /* not sure if this is needed...
        if ( num_particles_above_threshold_ == 0 ) 
            return currentTrackClassification;
         */

        if ( aTrack->GetTotalEnergy() < threshold_ ) 
            return fWaiting;

        return currentTrackClassification; 
    }

    void PartialEnergySorter::stepping(const G4Step* step) {

        if ( num_particles_above_threshold_ == 0 ) return;

        auto pre_energy{step->GetPreStepPoint()->GetTotalEnergy()};
        auto post_energy{step->GetPostStepPoint()->GetTotalEnergy()};

        if ( pre_energy >= threshold_ and post_energy <= threshold_ ) {
            num_particles_above_threshold_--;
            step->GetTrack()->SetTrackStatus(fSuspend);
        }
    } 

} // ldmx

DECLARE_ACTION(ldmx, PartialEnergySorter)
