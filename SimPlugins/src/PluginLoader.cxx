#include "SimPlugins/PluginLoader.h"

#include <dlfcn.h>

namespace sim {

UserActionPlugin* PluginLoader::create(std::string pluginName, std::string libName) {

    void* handle = dlopen(libName.c_str(), RTLD_LAZY);

    UserActionPlugin* (*create)();
    void (*destroy)(UserActionPlugin*);

    create = (UserActionPlugin* (*)())dlsym(handle, std::string("create" + pluginName).c_str());
    destroy = (void (*)(UserActionPlugin*))dlsym(handle, std::string("destroy" + pluginName).c_str());

    UserActionPlugin* plugin = (UserActionPlugin*)create();
    //destroy( myClass );
    return plugin;
}

}
