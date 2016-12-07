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

namespace sim {

EcalSD::EcalSD(G4String name, G4String theCollectionName, int subdetID, DetectorID* detID) :
		CalorimeterSD(name,theCollectionName,subdetID,detID){
	initialiseMap(EventConstants::ECAL_MAP_XY,EventConstants::CELL_SIZE);
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
    int cellID = map->FindBin(position[0],position[1]);
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

void EcalSD::initialiseMap(const double width, const double side) {
	map = new TH2Poly();
	// Center a cell at (x,y)=(0,0) and ensure coverage up to/past width/2 in all 4 directions,
	// assuming each cell is lying on a side.

	unsigned ncellwide = width / (2. * side);
	unsigned ny        = ncellwide + 1;
	unsigned nx        = ncellwide + 4;
	double   xstart    = -((double) ncellwide + 0.5) * side;
	double   ystart    = -((double) ncellwide + 1) * side * sqrt(3) / 2;

	if (this->verboseLevel > 0) {
		std::cout << " -- Initialising HoneyComb with parameters: " << std::endl
				<< " ---- (xstart,ystart) = (" << xstart << "," << ystart << ")"
				<< ", side = " << side << ", nx = " << nx << ", ny=" << ny
				<< std::endl;
	}
	buildMap( xstart, ystart, side, ny, nx);
}

void EcalSD::buildMap(Double_t xstart,
		Double_t ystart, Double_t a, Int_t k, Int_t s){
	// Add the bins
	Double_t numberOfHexagonsInAColumn;
	Double_t x[6], y[6];
	Double_t xloop, yloop, ytemp;
	Double_t sqrt_three = sqrt(3);
	xloop = xstart;
	yloop = ystart + a * sqrt_three / 2.0;

	for (int sCounter = 0; sCounter < s; sCounter++) {

		ytemp = yloop; // Resets the temp variable

		// Determine the number of hexagons in that column
		if (sCounter % 2 == 0) {
			numberOfHexagonsInAColumn = k;
		} else {
			numberOfHexagonsInAColumn = k - 1;
		}

		for (int kCounter = 0; kCounter < numberOfHexagonsInAColumn;
				kCounter++) {

			// Go around the hexagon
			x[0] = xloop;
			y[0] = ytemp;
			x[1] = x[0] + a / 2.0;
			y[1] = y[0] + a * sqrt_three / 2.0;
			x[2] = x[1] + a;
			y[2] = y[1];
			x[3] = x[2] + a / 2.0;
			y[3] = y[1] - a * sqrt_three / 2.0;
			;
			x[4] = x[2];
			y[4] = y[3] - a * sqrt_three / 2.0;
			;
			x[5] = x[1];
			y[5] = y[4];
			map->AddBin(6, x, y);

			// Go up
			ytemp += a * sqrt_three;
		}

		// Increment the starting position
		if (sCounter % 2 == 0)
			yloop += a * sqrt_three / 2.0;
		else
			yloop -= a * sqrt_three / 2.0;
		xloop += 1.5 * a;
		}
 }
}
