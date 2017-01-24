#include "SimPlugins/EventPrintPlugin.h"

/**
 * Entry point for creating a new EventPrintPlugin.
 */
extern "C" ldmx::EventPrintPlugin* createEventPrintPlugin() {
    return new ldmx::EventPrintPlugin;
}

/**
 * Entry point for destroying an existing EventPrintPlugin.
 */
extern "C" void destroyEventPrintPlugin(ldmx::EventPrintPlugin* object) {
    delete object;
}
