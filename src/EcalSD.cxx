#include "SimCore/EcalSD.h"

// Geant4
#include "G4Polyhedron.hh"
#include "G4VSolid.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4Geantino.hh"
#include "G4ChargedGeantino.hh"

/*~~~~~~~~~~~~~~*/
/*   DetDescr   */
/*~~~~~~~~~~~~~~*/
#include "DetDescr/EcalID.h" 

namespace ldmx {

    EcalSD::EcalSD(G4String name, G4String theCollectionName, int subDetID, ConditionsInterface& ci) :
      CalorimeterSD(name, theCollectionName), conditionsIntf_(ci) {
    }

    EcalSD::~EcalSD() {
    }

    G4bool EcalSD::ProcessHits(G4Step* aStep, G4TouchableHistory*) {


	const EcalHexReadout& hitMap = conditionsIntf_.getCondition<EcalHexReadout>(EcalHexReadout::CONDITIONS_OBJECT_NAME);
	
        // Determine if current particle of this step is a Geantino.
        G4ParticleDefinition* pdef = aStep->GetTrack()->GetDefinition();
        bool isGeantino = false;
        if (pdef == G4Geantino::Definition() || pdef == G4ChargedGeantino::Definition()) {
            isGeantino = true;
        }

        // Get the edep from the step.
        G4double edep = aStep->GetTotalEnergyDeposit();

        // Skip steps with no energy dep which come from non-Geantino particles.
        if (edep == 0.0 && !isGeantino) {
            if (verboseLevel > 2) {
                G4cout << "CalorimeterSD skipping step with zero edep." << G4endl << G4endl;
            }
            return false;
        }

        // Create a new cal hit.
        G4CalorimeterHit* hit = new G4CalorimeterHit();

        // Set the edep.
        hit->setEdep(edep);

        // Compute the hit position using the utility function.
        G4ThreeVector hitPosition = getHitPosition(aStep);
        hit->setPosition(hitPosition.x(), hitPosition.y(), hitPosition.z());

        // Set the global time.
        hit->setTime(aStep->GetTrack()->GetGlobalTime());

        // Create the ID for the hit.
        int cpynum = aStep->GetPreStepPoint()->GetTouchableHandle()->GetHistory()->GetVolume(layerDepth_)->GetCopyNo();
        int layerNumber;
        layerNumber = int(cpynum/7);
        int module_position = cpynum%7;

        EcalID partialId = hitMap.getCellModuleID(hitPosition[0], hitPosition[1]);
	    EcalID id(layerNumber, module_position, partialId.cell());
        hit->setID(id.raw());

        // Set the track ID on the hit.
        hit->setTrackID(aStep->GetTrack()->GetTrackID());

        // Set the PDG code from the track.
        hit->setPdgCode(aStep->GetTrack()->GetParticleDefinition()->GetPDGEncoding());

        if (this->verboseLevel > 2) {
	        G4cout << "Created new SimCalorimeterHit in detector " << this->GetName() << " with subdet ID " << id << " ...";
            hit->Print();
            G4cout << G4endl;
        }

        // Insert the hit into the hits collection.
        hitsCollection_->insert(hit);

        return true;
    }

    G4ThreeVector EcalSD::getHitPosition(G4Step* aStep) {

        /**
         * Set initial hit position from midpoint of the step.
         */
        G4StepPoint* prePoint = aStep->GetPreStepPoint();
        G4StepPoint* postPoint = aStep->GetPostStepPoint();
        G4ThreeVector position = 0.5 * (prePoint->GetPosition() + postPoint->GetPosition());

        /*
         * Get the volume position in global coordinates, which for the ECal is the center of
         * the front face of the sensor.
         */
        G4ThreeVector volumePosition = aStep->GetPreStepPoint()->GetTouchableHandle()->GetHistory()->GetTopTransform().Inverse().TransformPoint(G4ThreeVector());

        // Get the solid from this step.
        G4VSolid* solid = prePoint->GetTouchableHandle()->GetVolume()->GetLogicalVolume()->GetSolid();
        auto it = polyMap_.find(solid);
        G4Polyhedron* poly;
        if (it == polyMap_.end()) {
            poly = solid->CreatePolyhedron();
            polyMap_[solid] = poly;
        } else {
            poly = polyMap_[solid];
        }

        /**
         * Use facet info of the solid for setting Z to the sensor midpoint.
         */
        G4Point3D iNodes[4];
        G4int n;
        poly->GetFacet(1, n, iNodes);
        G4double zstart = iNodes[1][2];
        G4double zend = iNodes[0][2];
        G4double zmid = (zstart - zend) / 2;
        position.setZ(volumePosition.z() + zmid);

        return position;
    }

}
