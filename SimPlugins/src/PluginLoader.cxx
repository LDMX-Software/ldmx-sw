#include "SimPlugins/PluginLoader.h"

#include <dlfcn.h>

namespace sim {

UserActionPlugin* PluginLoader::create(std::string pluginName, std::string libName) {

    void* handle = dlopen(libName.c_str(), RTLD_LAZY);

    UserActionPlugin* (*createIt)();
    createIt = (UserActionPlugin* (*)())dlsym(handle, std::string("create" + pluginName).c_str());
    UserActionPlugin* plugin = (UserActionPlugin*)createIt();

    this->pluginHandles[plugin] = handle;

    return plugin;
}

void PluginLoader::destroy(UserActionPlugin* plugin) {

    void* handle = this->pluginHandles[plugin];

    void (*destroyIt)(UserActionPlugin*);
    destroyIt = (void (*)(UserActionPlugin*))dlsym(handle, std::string("destroy" + plugin->getName()).c_str());
    destroyIt(plugin);

    std::map<UserActionPlugin*, void*>::iterator it;
    it = this->pluginHandles.find(plugin);
    this->pluginHandles.erase(it);
}

}
