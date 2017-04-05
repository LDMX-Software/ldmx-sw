/**
 * @file TrackKiller.cxx
 * @class TrackKiller
 * @brief Class defining a UserActionPlugin that allows a user to kill all 
 *        tracks in an event.  This is used for testing purposes only.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimPlugins/TrackKiller.h"

namespace ldmx {

    extern "C" TrackKiller* createTrackKiller() {
        return new TrackKiller;
    }

    extern "C" void destroyTrackKiller(TrackKiller* object) {
        delete object;
    }

    TrackKiller::TrackKiller() {
    }

    TrackKiller::~TrackKiller() {
    }

    void TrackKiller::stepping(const G4Step* step) {

        // Get the track associated with this step.
        G4Track* track = step->GetTrack();

        track->SetTrackStatus(fKillTrackAndSecondaries);
        G4RunManager::GetRunManager()->AbortEvent();
        return;
    }
}

