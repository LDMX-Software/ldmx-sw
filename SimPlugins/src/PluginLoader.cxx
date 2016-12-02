#include "SimPlugins/PluginLoader.h"

#include <dlfcn.h>

namespace sim {

UserActionPlugin* PluginLoader::create(std::string pluginName, std::string libName) {

    // Open a handle to the specific dynamic lib.
    void* handle = dlopen(libName.c_str(), RTLD_LAZY);

    // Plugin to be created.
    UserActionPlugin* plugin = nullptr;

    // Is there a proper handle to the lib?
    if (handle) {

        UserActionPlugin* (*createIt)();
        createIt = (UserActionPlugin* (*)())dlsym(handle, std::string("create" + pluginName).c_str());

        // Did we get back a good function pointer from the lib?
        if(createIt) {
            plugin = (UserActionPlugin*)createIt();

            // Was plugin created successfully?
            if (plugin) {
                this->pluginHandles_[plugin] = handle;
            } else {
                // For some reason, the plugin could not be created by the lib function!
                std::cerr << "[ PluginLoader ] : Failed to create plugin " << pluginName << " from lib "
                        << libName << "!!!" << std::endl;
                throw std::runtime_error("Failed to create the plugin.");
            }
        } else {
            // The create function was not found in the lib.
            std::cerr << "[ PluginLoader ] : Failed to find create function for plugin " << pluginName << " in lib "
                    << libName << "!!!" << std::endl;
            throw std::runtime_error("Failed to find library function for creating plugin.");
        }
    } else {
        // The plugin lib could not be opened.  Probably it doesn't exist!
        std::cerr << "[ PluginLoader ] : Could not open " << libName << " plugin lib." << std::endl;
        throw std::runtime_error("Plugin lib not found.");
    }

    return plugin;
}

void PluginLoader::destroy(UserActionPlugin* plugin) {

    // Is the plugin argument non-null?
    if (plugin) {

        // Get the lib handle for the plugin.
        void* handle = this->pluginHandles_[plugin];

        // Do we have a lib handle for this plugin?
        if (handle) {

            // Destroy the plugin by calling its destroy method from the lib.
            void (*destroyIt)(UserActionPlugin*);
            destroyIt = (void (*)(UserActionPlugin*))dlsym(handle, std::string("destroy" + plugin->getName()).c_str());
            destroyIt(plugin);

            // Delete from plugin handles.
            std::map<UserActionPlugin*, void*>::iterator it;
            it = this->pluginHandles_.find(plugin);
            if (it != pluginHandles_.end()) {
                this->pluginHandles_.erase(it);
            }
        } else {
            // No handle exists for this plugin.  Maybe it wasn't registered with this PluginLoader?
            std::cerr << "[ PluginLoader ] : Failed to find handle for " << plugin->getName() << " plugin!!!" << std::endl;
            throw std::runtime_error("Failed to find handle for plugin.");
        }
    } else {
        // Null argument is just ignored with a warning.
        std::cerr << "[ PluginLoader ] : Ignoring null plugin argument. " << std::endl;
    }
}

}
