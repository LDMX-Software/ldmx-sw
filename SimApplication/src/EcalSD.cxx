#include "SimApplication/EcalSD.h"

// STL
#include <iostream>

// Geant4
#include "G4Polyhedron.hh"
#include "G4VSolid.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4Geantino.hh"
#include "G4ChargedGeantino.hh"

namespace ldmx {

    EcalSD::EcalSD(G4String name, G4String theCollectionName, int subdetID, DetectorID* detID) :
            CalorimeterSD(name, theCollectionName, subdetID, detID) {

        // These are the v12 parameters
        //  all distances in mm
        double moduleRadius = 85.0; //same as default
        int    numCellsWide = 23; //same as default
        double moduleGap = 1.0;
        double ecalFrontZ = 220;
        std::vector<double> ecalSensLayersZ = {
             7.850,
            13.300,
            26.400,
            33.500,
            47.950,
            56.550,
            72.250,
            81.350,
            97.050,
            106.150,
            121.850,
            130.950,
            146.650,
            155.750,
            171.450,
            180.550,
            196.250,
            205.350,
            221.050,
            230.150,
            245.850,
            254.950,
            270.650,
            279.750,
            298.950,
            311.550,
            330.750,
            343.350,
            362.550,
            375.150,
            394.350,
            406.950,
            426.150,
            438.750
        };

        hitMap_ = std::make_unique<EcalHexReadout>(
                moduleRadius,
                moduleGap,
                numCellsWide,
                ecalSensLayersZ,
                ecalFrontZ
                );
    }

    EcalSD::~EcalSD() {
    }

    G4bool EcalSD::ProcessHits(G4Step* aStep, G4TouchableHistory*) {

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
                std::cout << "CalorimeterSD skipping step with zero edep." << std::endl << std::endl;
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

        int cellModuleID = hitMap_->getCellModuleID(hitPosition[0], hitPosition[1]);
        int cellID = (hitMap_->separateID(cellModuleID)).first;
        detID_->setFieldValue(1, layerNumber);
        detID_->setFieldValue(2, module_position);
        detID_->setFieldValue(3, cellID);
        hit->setID(detID_->pack());

        // Set the track ID on the hit.
        hit->setTrackID(aStep->GetTrack()->GetTrackID());

        // Set the PDG code from the track.
        hit->setPdgCode(aStep->GetTrack()->GetParticleDefinition()->GetPDGEncoding());

        if (this->verboseLevel > 2) {
        std::cout << "Created new SimCalorimeterHit in detector " << this->GetName() << " with subdet ID " << subdet_ << " and layer " << layerNumber << " and cellid " << cellID << " module position " << module_position << " ...";
            hit->Print();
            std::cout << std::endl;
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
