#include "SimApplication/G4CalorimeterHit.h"

// Geant4
#include "G4VVisManager.hh"
#include "G4VisAttributes.hh"
#include "G4Point3D.hh"
#include "G4Circle.hh"

namespace sim {

G4Allocator<G4CalorimeterHit> G4CalorimeterHitAllocator;

void G4CalorimeterHit::Draw() {

    G4VVisManager* visManager = G4VVisManager::GetConcreteInstance();

    if(visManager) {
        G4Point3D p3D = G4Point3D(position_);
        G4Circle chit(p3D);
        chit.SetScreenDiameter(3.0);
        chit.SetFillStyle(G4Circle::filled);

        G4Colour col(1.0, 0.0, 1.0);

        chit.SetVisAttributes(G4VisAttributes(col));
        visManager->Draw(chit);
    }
}

void G4CalorimeterHit::Print() {
    print(std::cout);
}

std::ostream& G4CalorimeterHit::print(std::ostream& os) {
    os << "G4CalorimeterHit { "
            << "edep: " << this->edep_ << ", "
            << "position: " << position_ << ", "
            << "time: " << this->time_
            << " }"
            << std::endl;
    return os;
}

void G4CalorimeterHit::updateSimCalorimeterHit(SimCalorimeterHit* simCalHit, bool existingHit) {
    // Update an existing hit.
    if (existingHit) {

        // Increment the edep.
        simCalHit->setEdep(edep_ + simCalHit->getEdep());

        // Set time if additional hit's time is earlier.
        if (simCalHit->getTime() < time_) {
            time_ = simCalHit->getTime();
        }
    // Create a new hit.
    } else {
        simCalHit->setID(id_);
        simCalHit->setEdep(edep_);
        simCalHit->setPosition(position_.x(), position_.y(), position_.z());
        simCalHit->setTime(time_);
        this->simCalHit_ = simCalHit;
    }
}

}
