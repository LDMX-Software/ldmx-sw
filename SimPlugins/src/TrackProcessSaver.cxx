#include "SimPlugins/TrackProcessSaver.h"

#include "SimPlugins/TrackProcessSaverMessenger.h"

#include "G4VProcess.hh"

namespace ldmx { 

    TrackProcessSaver::TrackProcessSaver() {
        messenger_ = new TrackProcessSaverMessenger(this);
    }

    TrackProcessSaver::~TrackProcessSaver() {
        delete messenger_;
    }

    void TrackProcessSaver::preTracking(const G4Track* aTrack) {
        auto trackInfo = dynamic_cast<UserTrackInformation*>(aTrack->GetUserInformation());
        const G4VProcess* process = aTrack->GetCreatorProcess();
        if (process) {
            if (processNames_.find(process->GetProcessName()) != processNames_.end()) {
                trackInfo->setSaveFlag(true);
                if (verbose_ > 1) {
                    std::cout << "[ TrackProcessSaver ] : saving track ID " << aTrack->GetTrackID()
                            << " with process '" << process->GetProcessName() << "'" << std::endl;
                }
            }
        }
    }

    extern "C" TrackProcessSaver* createTrackProcessSaver() {
        return new TrackProcessSaver;
    }

    extern "C" void destroyTrackProcessSaver(TrackProcessSaver* object) {
        delete object;
    }
}

