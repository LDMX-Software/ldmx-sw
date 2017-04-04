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
            for (const auto& processName : processNames_) {
                if (exactMatch_[processName]) {
                    if (!processName.compare(process->GetProcessName())) {
                        trackInfo->setSaveFlag(true);
                    }
                } else {
                    if (process->GetProcessName().find(processName) != std::string::npos) {
                        trackInfo->setSaveFlag(true);
                    }
                }
            }
            if (trackInfo->getSaveFlag()) {
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

