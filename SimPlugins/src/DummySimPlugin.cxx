#include "SimPlugins/DummySimPlugin.h"

/**
 * Entry point for creating a new plugin.
 */
extern "C" ldmx::DummySimPlugin* createDummySimPlugin() {
    return new ldmx::DummySimPlugin;
}

/**
 * Entry point for destroying an existing plugin.
 */
extern "C" void destroyDummySimPlugin(ldmx::DummySimPlugin* object) {
    delete object;
}
