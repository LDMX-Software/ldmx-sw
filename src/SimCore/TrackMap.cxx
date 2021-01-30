#include "SimCore/TrackMap.h"

// Geant4
#include "G4Event.hh"
#include "G4EventManager.hh"

// LDMX
#include "SimCore/Trajectory.h"

namespace simcore {

int TrackMap::findIncident(G4int trackID) {
  int currTrackID = trackID;
  bool foundIncident = false;
  while (not foundIncident) {
    if (this->contains(currTrackID)) {
      // currTrackID is not a primary and has parent stored
      currTrackID = this->getParent(currTrackID);
      if (this->hasTrajectory(currTrackID)) {
        // curr track ID is being stored
        auto region{this->getTrajectory(currTrackID)->getVertexRegion()};
        if (region.find("Calorimeter") == std::string::npos) {
          // curr track originated outside cal region
          foundIncident = true;
        }
      }
    } else {
      // curr Track ID is a primary and has already been
      // checked above, give up
      foundIncident = true;
    }
  }
  return currTrackID;
}

void TrackMap::clear() {
  trajectoryMap_.clear();
  trackIDMap_.clear();
}
}  // namespace simcore
