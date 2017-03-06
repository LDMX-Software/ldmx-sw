#include "SimPlugins/TrackSaver.h"

namespace ldmx { 

    extern "C" TrackSaver* createTrackSaver() {
        return new TrackSaver;
    }

    extern "C" void destroyTrackSaver(TrackSaver* object) {
        delete object;
    }
}

