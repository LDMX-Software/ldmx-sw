#ifndef SIMAPPLICATION_PLUGINMANAGERACCESSOR_H
#define SIMAPPLICATION_PLUGINMANAGERACCESSOR_H

#include "SimPlugins/PluginManager.h"

namespace sim {

class PluginManagerAccessor {

    public:

        void setPluginManager(PluginManager* pluginManager) {
            pluginManager_ = pluginManager;
        }

        PluginManager* getPluginManager() {
            return pluginManager_;
        }

    protected:

        /* Allow protected access for convenience of sub-classes. */
        PluginManager* pluginManager_;
};

}

#endif
