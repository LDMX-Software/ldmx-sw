#include "SimCore/G4TrackerHit.h"

// Geant4
#include "G4Circle.hh"
#include "G4Point3D.hh"
#include "G4VVisManager.hh"
#include "G4VisAttributes.hh"

namespace simcore {

G4Allocator<G4TrackerHit> G4TrackerHitAllocator;

void G4TrackerHit::Draw() {
  G4VVisManager* visManager = G4VVisManager::GetConcreteInstance();

  if (visManager) {
    G4Point3D p3D = G4Point3D(this->position_);
    G4Circle chit(p3D);
    chit.SetScreenDiameter(3.0);
    chit.SetFillStyle(G4Circle::filled);

    G4Colour col(1.0, 0.0, 1.0);

    chit.SetVisAttributes(G4VisAttributes(col));
    visManager->Draw(chit);
  }
}

void G4TrackerHit::Print() { print(std::cout); }

std::ostream& G4TrackerHit::print(std::ostream& os) {
  os << "G4TrackerHit { "
        "edep: "
     << this->edep_
     << ", "
        "position: ("
     << this->position_[0] << ", " << this->position_[1] << ", "
     << this->position_[2] << "), "
     << "layerID: " << this->layerID_ << ", "
     << "momentum: (" << this->momentum_[0] << ", " << this->momentum_[1]
     << ", " << this->momentum_[2] << "), "
     << "pathLength: " << this->pathLength_ << ", "
     << "time: " << this->time_ << " }" << std::endl;
  return os;
}

}  // namespace simcore
