#include "SimPlugins/DummySimPlugin.h"

/**
 * Entry point for creating a new plugin.
 */
extern "C" sim::DummySimPlugin* createDummySimPlugin() {
    return new sim::DummySimPlugin;
}

/**
 * Entry point for destroying an existing plugin.
 */
extern "C" void destroyDummySimPlugin(sim::DummySimPlugin* object) {
    delete object;
}
