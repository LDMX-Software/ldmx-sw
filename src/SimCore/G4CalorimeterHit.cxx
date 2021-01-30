#include "SimCore/G4CalorimeterHit.h"

// Geant4
#include "G4Circle.hh"
#include "G4Point3D.hh"
#include "G4VVisManager.hh"
#include "G4VisAttributes.hh"

namespace simcore {

G4Allocator<G4CalorimeterHit> G4CalorimeterHitAllocator;

void G4CalorimeterHit::Draw() {
  G4VVisManager* visManager = G4VVisManager::GetConcreteInstance();

  if (visManager) {
    G4Point3D p3D = G4Point3D(position_);
    G4Circle chit(p3D);
    chit.SetScreenDiameter(3.0);
    chit.SetFillStyle(G4Circle::filled);

    G4Colour col(1.0, 0.0, 1.0);

    chit.SetVisAttributes(G4VisAttributes(col));
    visManager->Draw(chit);
  }
}

void G4CalorimeterHit::Print() { print(std::cout); }

std::ostream& G4CalorimeterHit::print(std::ostream& os) {
  os << "G4CalorimeterHit { "
     << "edep: " << this->edep_ << ", "
     << "position: " << position_ << ", "
     << "time: " << this->time_ << " }" << std::endl;
  return os;
}

}  // namespace simcore
