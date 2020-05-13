#include "SimApplication/TrigScintSD.h" 

/*~~~~~~~~~~~~~~*/
/*   DetDescr   */
/*~~~~~~~~~~~~~~*/
#include "DetDescr/DetectorID.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4ChargedGeantino.hh"
#include "G4Geantino.hh" 
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"

namespace ldmx { 

    TrigScintSD::TrigScintSD(G4String name, G4String theCollectionName, int subdet, DetectorID* detID) 
        : CalorimeterSD(name, theCollectionName, subdet, detID) {
    
        // Add the collection name to vector of names.
        collectionName.push_back(theCollectionName);

        // Register this SD with the manager.
        G4SDManager::GetSDMpointer()->AddNewDetector(this);

        // Set the subdet ID as it will always be the same for every hit.
        detID_->setFieldValue("subdet", subdet_);
    }

    TrigScintSD::~TrigScintSD() {
       delete detID_; 
    }

    G4bool TrigScintSD::ProcessHits(G4Step* step, G4TouchableHistory* history) {
       
        // Get the energy deposited by the particle during the step
        auto energy{step->GetTotalEnergyDeposit()}; 

        // If a non-Geantino particle doesn't deposit energy during the step, 
        // skip processing it.
        if (auto particleDef{step->GetTrack()->GetDefinition()}; 
                (energy == 0) 
                && ((particleDef != G4Geantino::Definition()) 
                        || (particleDef != G4ChargedGeantino::Definition()))) return false;

        // Create a new instance of a calorimeter hit
        auto hit{new G4CalorimeterHit()};

        // Set the energy deposition
        hit->setEdep(energy); 

        // Set the hit position
        auto position{0.5 * (step->GetPreStepPoint()->GetPosition() + step->GetPostStepPoint()->GetPosition())};
        auto volumePosition{step->GetPreStepPoint()->GetTouchableHandle()->GetHistory()->GetTopTransform().Inverse().TransformPoint(G4ThreeVector())};
        hit->setPosition(position[0], position[1], volumePosition.z());

        // Get the track associated with this step
        auto track{step->GetTrack()}; 

        // Set the global time.
        hit->setTime(track->GetGlobalTime());

        // Set the ID on the hit.
        auto layerNumber{track->GetVolume()->GetCopyNo()};
        detID_->setFieldValue(1, layerNumber);
        hit->setID(detID_->pack());

        // Set the track ID on the hit.
        hit->setTrackID(track->GetTrackID());

        // Set the PDG code from the track.
        hit->setPdgCode(track->GetParticleDefinition()->GetPDGEncoding());

        hitsCollection_->insert(hit);

        return true;
    }
}
