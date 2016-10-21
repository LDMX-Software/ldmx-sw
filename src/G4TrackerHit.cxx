#include "SimApplication/G4TrackerHit.h"

// Geant4
#include "G4VVisManager.hh"
#include "G4VisAttributes.hh"
#include "G4Point3D.hh"
#include "G4Circle.hh"

namespace sim {

G4Allocator<G4TrackerHit> G4TrackerHitAllocator;

void G4TrackerHit::Draw() {

    G4VVisManager* visManager = G4VVisManager::GetConcreteInstance();

    if(visManager) {

        G4Point3D p3D = G4Point3D(this->position);
        G4Circle chit(p3D);
        chit.SetScreenDiameter(3.0);
        chit.SetFillStyle(G4Circle::filled);

        G4Colour col(1.0, 0.0, 1.0);

        chit.SetVisAttributes(G4VisAttributes(col));
        visManager->Draw(chit);
    }
}

}
