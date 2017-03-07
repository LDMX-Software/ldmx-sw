#include "SimPlugins/PNTrackSaver.h"

namespace ldmx { 

    extern "C" PNTrackSaver* createPNTrackSaver() {
        return new PNTrackSaver;
    }

    extern "C" void destroyPNTrackSaver(PNTrackSaver* object) {
        delete object;
    }
}

