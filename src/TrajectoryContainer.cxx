#include "SimApplication/TrajectoryContainer.h"

namespace ldmx {

    Trajectory* TrajectoryContainer::findByTrackID(G4int trackID) {
        Trajectory* traj = NULL;
        for (int iTraj = 0; iTraj < this->entries(); iTraj++) {
            if ((*this)[iTraj]->GetTrackID() == trackID) {
                traj = (Trajectory*) (*this)[iTraj];
                break;
            }
        }
        return traj;
    }

}
