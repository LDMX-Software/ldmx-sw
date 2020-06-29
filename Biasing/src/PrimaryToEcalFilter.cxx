
#include "Biasing/PrimaryToEcalFilter.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4RunManager.hh" 
#include "G4Step.hh"

namespace ldmx {

    PrimaryToEcalFilter::PrimaryToEcalFilter(const std::string& name, Parameters& parameters) 
        : UserAction(name, parameters) {
            threshold_ = parameters.getParameter< double >("threshold"); 
    }

    PrimaryToEcalFilter::~PrimaryToEcalFilter() {
    } 

    void PrimaryToEcalFilter::stepping(const G4Step* step) {

        // Only process the primary electron track
        if (int parentID{step->GetTrack()->GetParentID()}; parentID != 0) return;

        // Get the region the particle is currently in.  Continue processing
        // the particle only if it's NOT in the calorimeter region
        if (auto region{step->GetTrack()->GetVolume()->GetLogicalVolume()->GetRegion()->GetName()};
                region.compareTo("CalorimeterRegion") == 0) return; 

        // If the energy of the particle fell below threshold, stop processing the event.
        if (auto energy{step->GetPostStepPoint()->GetTotalEnergy()}; energy < threshold_) { 
            step->GetTrack()->SetTrackStatus(fKillTrackAndSecondaries); 
            G4RunManager::GetRunManager()->AbortEvent(); 
            return; 
        }
    } 

} // ldmx

DECLARE_ACTION(ldmx, PrimaryToEcalFilter)
