#include "SimPlugins/PluginLoader.h"

#include "Exception/Exception.h"

#include <string>
#include <dlfcn.h>

namespace ldmx {

    UserActionPlugin* PluginLoader::create(std::string pluginName, std::string libName) {

        // Open a handle to the specific dynamic lib.
        void* handle = dlopen(libName.c_str(), RTLD_LAZY);

        // Plugin to be created.
        UserActionPlugin* plugin = nullptr;

        // Is there a proper handle to the lib?
        if (handle) {

            // Get a function pointer from the library to create the plugin object.
            UserActionPlugin* (*createIt)();
            createIt = (UserActionPlugin* (*)())dlsym(handle, std::string("create" + pluginName).c_str());

            // Did we get back a good function pointer from the lib?
            if(createIt) {

                // Create the plugin object from the function pointer.
                plugin = (UserActionPlugin*)createIt();

                // Was the plugin created successfully?
                if (plugin) {
                    // Register the plugin handle for when it needs to be destroyed.
                    this->pluginHandles_[plugin] = handle;
                } else {
                    // For some reason, the plugin could not be created by the lib function!
                    EXCEPTION_RAISE( "CreateFail" , "Failed to create plugin " + pluginName
                            + " from lib " + libName + "." );
                }
            } else {
                // The create function was not found in the lib.
                EXCEPTION_RAISE( "CreateFail" , "Failed to find create function for plugin " + pluginName
                        + " in lib " + libName + "." );
            }
        } else {
            // The plugin lib could not be opened.  Probably it doesn't exist!
            EXCEPTION_RAISE( "CreateFail" , "Could not open " + libName + " plugin lib." );
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
                destroyIt = (void (*)(UserActionPlugin*))dlsym(handle, std::string("destroy" + plugin->getName()).c_str());destroyIt
                (plugin);

                // Delete from plugin handles.
                std::map<UserActionPlugin*, void*>::iterator it;
                it = this->pluginHandles_.find(plugin);
                if (it != pluginHandles_.end()) {
                    this->pluginHandles_.erase(it);
                }
            } else {
                // No handle exists for this plugin.  Maybe it wasn't registered with this PluginLoader?
                EXCEPTION_RAISE( "FindFail" , "Failed to find handle for " + plugin->getName() + " plugin." );
            }
        } else {
            // Null argument is just ignored with a warning.
            std::cerr << "[ PluginLoader ] : Ignoring null plugin argument. " << std::endl;
        }
    }

}
