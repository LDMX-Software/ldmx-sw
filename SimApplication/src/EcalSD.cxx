#include "SimApplication/EcalSD.h"

// STL
#include <iostream>

// Geant4
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4Geantino.hh"
#include "G4ChargedGeantino.hh"

// LDMX
#include "Event/RootEventWriter.h"
#include "Event/EventConstants.h"

using event::EventConstants;
using event::RootEventWriter;
using detdescr::EcalHexReadout;

namespace sim {

EcalSD::EcalSD(G4String name, G4String theCollectionName, int subdetID, DetectorID* detID ) :
		CalorimeterSD(name,theCollectionName,subdetID,detID){
    hitMap = new EcalHexReadout();
};

EcalSD::~EcalSD() {}

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

    // Create a new hit object using the ROOT event.
    //SimCalorimeterHit* simCalorimeterHit =
    //        (SimCalorimeterHit*) currentEvent->addObject(collectionName[0]);
    G4CalorimeterHit* hit = new G4CalorimeterHit(/*simCalorimeterHit*/);

    // Set the edep.
    hit->setEdep(edep);

    // Set the position.
    G4StepPoint* prePoint = aStep->GetPreStepPoint();
    G4StepPoint* postPoint = aStep->GetPostStepPoint();
    G4ThreeVector position = 0.5 * (prePoint->GetPosition() + postPoint->GetPosition());
    G4ThreeVector volumePosition = aStep->GetPreStepPoint()->GetTouchableHandle()->GetHistory()
            ->GetTopTransform().Inverse().TransformPoint(G4ThreeVector());
    hit->setPosition(position[0], position[1], volumePosition.z());

    // Set the global time.
    hit->setTime(aStep->GetTrack()->GetGlobalTime());

    // Set the ID on the hit.
    int layerNumber = prePoint->GetTouchableHandle()->GetHistory()->GetVolume(layerDepth_)->GetCopyNo();
    int cellID = hitMap->getCellId(position[0],position[1]);
    detID_->setFieldValue(1, layerNumber);
    detID_->setFieldValue(2, cellID);
    hit->setID(detID_->pack());

    // Set the track ID on the hit.
    hit->setTrackID(aStep->GetTrack()->GetTrackID());

    if (this->verboseLevel > 2) {
        std::cout << "Created new SimCalorimeterHit in detector " << this->GetName()
                << " with subdet ID " << subdet_ << " and layer " << layerNumber << " and cellid " << cellID << " ..." ;
        hit->Print();
        std::cout << std::endl;
    }

    hitsCollection_->insert(hit);

    return true;
}

}
