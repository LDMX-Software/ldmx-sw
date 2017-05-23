#include "SimPlugins/TrackFilterPlugin.h"

namespace ldmx {

    extern "C" TrackFilterPlugin* createTrackFilterPlugin() {
        return new TrackFilterPlugin;
    }

    extern "C" void destroyTrackFilterPlugin(TrackFilterPlugin* object) {
        delete object;
    }

}
