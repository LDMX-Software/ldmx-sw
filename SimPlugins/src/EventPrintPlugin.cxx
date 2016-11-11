#include "SimPlugins/EventPrintPlugin.h"

/**
 * Entry point for creating a new EventPrintPlugin.
 */
extern "C" sim::EventPrintPlugin* createEventPrintPlugin() {
    return new sim::EventPrintPlugin;
}

/**
 * Entry point for destroying an existing EventPrintPlugin.
 */
extern "C" void destroyEventPrintPlugin(sim::EventPrintPlugin* object) {
    delete object;
}
