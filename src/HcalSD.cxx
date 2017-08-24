#include "SimApplication/HcalSD.h"

// STL
#include <iostream>

// Geant4
#include "G4Box.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4SDManager.hh"
#include "G4Geantino.hh"
#include "G4ChargedGeantino.hh"

namespace ldmx {

    HcalSD::HcalSD(G4String name, G4String theCollectionName, int subdetID, DetectorID* detID) :
            CalorimeterSD(name, theCollectionName, subdetID, detID) {
    }

    HcalSD::~HcalSD() {}

    G4bool HcalSD::ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) {

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
 
        // Get the scintillator solid box
        G4Box* scint = static_cast<G4Box*>(aStep->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume()->GetSolid());

        // Set the step mid-point as the hit position.
        G4StepPoint* prePoint = aStep->GetPreStepPoint();
        G4StepPoint* postPoint = aStep->GetPostStepPoint();
        G4ThreeVector position = 0.5 * (prePoint->GetPosition() + postPoint->GetPosition());
        G4ThreeVector localPosition = aStep->GetPreStepPoint()->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(position);        
        hit->setPosition(position[0], position[1], position[2]);
        
        //to be copied after section definition
        // G4ThreeVector volumePosition = aStep->GetPreStepPoint()->GetTouchableHandle()->GetHistory()->GetTopTransform().Inverse().TransformPoint(G4ThreeVector());
        // if (section==HcalSection::BACK) hit->setPosition(position[0], position[1], volumePosition.z());
        // elseif (section==HcalSection::TOP || section==HcalSection::BOTTOM) hit->setPosition(position[0], volumePosition.y(), position[2]);
        // elseif (section==HcalSection::LEFT || section==HcalSection::RIGHT) hit->setPosition(volumePosition.x(),position[1] , position[2]);        

        // Set the global time.
        hit->setTime(aStep->GetTrack()->GetGlobalTime());

        // Create the ID for the hit.
        int copyNum = aStep->GetPreStepPoint()->GetTouchableHandle()->GetHistory()->GetVolume(layerDepth_)->GetCopyNo();
        int section = copyNum / 1000;
        int layer = copyNum % 1000;

        //stripID: back Hcal, segmented along y direction for now every 10 cm -- alternate x-y in the future?
        //         left/right side hcal: segmented along x direction every 10 cm
        //         top/bottom side hcal: segmented along y direction every 10 cm
        int stripID = -1;        
        if (section==HcalSection::BACK) stripID = int( (localPosition.y()+scint->GetYHalfLength())/100.0);
        else if (section==HcalSection::TOP || section==HcalSection::BOTTOM) stripID = int( (localPosition.x()+scint->GetXHalfLength())/100.0);
        else if (section==HcalSection::LEFT || section==HcalSection::RIGHT) stripID = int( (localPosition.y()+scint->GetYHalfLength())/100.0);


        detID_->setFieldValue(1, layer);
        detID_->setFieldValue(2, section);
        detID_->setFieldValue(3, stripID);
        hit->setID(detID_->pack());

        // Set the track ID on the hit.
        hit->setTrackID(aStep->GetTrack()->GetTrackID());

        // Set the PDG code from the track.
        hit->setPdgCode(aStep->GetTrack()->GetParticleDefinition()->GetPDGEncoding());

        if (this->verboseLevel > 2) {
            std::cout << "Created new SimCalorimeterHit in detector " << this->GetName()
                      << " subdet ID <" << subdet_ << ">, layer <" << layer << "> and section <" << section << ">, copynum <" << copyNum << ">"
                      << std::endl;
            hit->Print();
            std::cout << std::endl;
        }

        // Insert the hit into the hits collection.
        hitsCollection_->insert(hit);

        return true;
    }
}
