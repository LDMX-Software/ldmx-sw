#include "SimApplication/G4CalorimeterHit.h"

// Geant4
#include "G4VVisManager.hh"
#include "G4VisAttributes.hh"
#include "G4Point3D.hh"
#include "G4Circle.hh"

G4Allocator<G4CalorimeterHit> G4CalorimeterHitAllocator;

G4CalorimeterHit::G4CalorimeterHit() :
        simCalorimeterHit(new SimCalorimeterHit()), trackID(-1) {
}

G4CalorimeterHit::G4CalorimeterHit(SimCalorimeterHit* theSimCalorimeterHit) :
        simCalorimeterHit(theSimCalorimeterHit), trackID(-1) {
}

G4CalorimeterHit::~G4CalorimeterHit() {
}

SimCalorimeterHit* G4CalorimeterHit::getSimCalorimeterHit() {
    return simCalorimeterHit;
}

void G4CalorimeterHit::Draw() {

    G4VVisManager* visManager = G4VVisManager::GetConcreteInstance();

    if(visManager) {
        G4ThreeVector position(
                simCalorimeterHit->getPosition()[0],
                simCalorimeterHit->getPosition()[1],
                simCalorimeterHit->getPosition()[2]);
        G4Point3D p3D = G4Point3D(position);
        G4Circle chit(p3D);
        chit.SetScreenDiameter(3.0);
        chit.SetFillStyle(G4Circle::filled);

        G4Colour col(1.0, 0.0, 1.0);

        chit.SetVisAttributes(G4VisAttributes(col));
        visManager->Draw(chit);
    }
}

void G4CalorimeterHit::Print() {
    simCalorimeterHit->Print();
}

void G4CalorimeterHit::setTrackID(G4int aTrackID) {
    trackID = aTrackID;
}

G4int G4CalorimeterHit::getTrackID() {
    return trackID;
}
