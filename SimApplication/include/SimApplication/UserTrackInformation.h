#ifndef SIMAPPLICATION_USERTRACKINFORMATION_H_
#define SIMAPPLICATION_USERTRACKINFORMATION_H_ 1

// Geant4
//#include "G4Allocator.hh"
#include "G4VUserTrackInformation.hh"
#include "G4Track.hh"

// LDMX
#include "SimApplication/TrackSummary.h"

class UserTrackInformation : public G4VUserTrackInformation {

    public:

        UserTrackInformation(const G4Track* aTrack) {
            trackSummary = new TrackSummary(aTrack);
        }

        virtual ~UserTrackInformation() {
        }

        TrackSummary* getTrackSummary() {
            return trackSummary;
        }

        inline void *operator new(size_t);

        inline void operator delete(void*);

    private:

        TrackSummary* trackSummary;
};

extern G4Allocator<UserTrackInformation> UserTrackInformationAllocator;

inline void* UserTrackInformation::operator new(size_t) {
    void* trackInfo;
    trackInfo = (void*) UserTrackInformationAllocator.MallocSingle();
    return trackInfo;
}

inline void UserTrackInformation::operator delete(void *trackInfo) {
    UserTrackInformationAllocator.FreeSingle((UserTrackInformation*) trackInfo);
}

#endif

