/**
 * @file PluginManagerAccessor.h
 * @brief Mixin class for accessing the plugin manager
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_PLUGINMANAGERACCESSOR_H
#define SIMAPPLICATION_PLUGINMANAGERACCESSOR_H

// LDMX
#include "SimPlugins/PluginManager.h"

namespace ldmx {

    /**
     * @class PluginManagerAccessor
     * @brief Mixin for accessing the PluginManager
     *
     * @note
     * This class is used to assign a plugin manager to each of the user action classes.
     */
    class PluginManagerAccessor {

        public:

            /**
             * Set the plugin manager pointer.
             * @param pluginManager Pointer to the plugin manager.
             */
            void setPluginManager(PluginManager* pluginManager) {
                pluginManager_ = pluginManager;
            }

            /**
             * Get the plugin manager.
             * @return The plugin manager.
             */
            PluginManager* getPluginManager() {
                return pluginManager_;
            }

        protected:

            /* The plugin manager pointer; allow protected access for convenience of sub-classes. */
            PluginManager* pluginManager_;
    };

}

#endif
