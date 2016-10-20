#ifndef SimPlugins_PluginLoader_h
#define SimPlugins_PluginLoader_h

#include "SimPlugins/UserActionPlugin.h"

namespace sim {

class PluginLoader {

    public:

        UserActionPlugin* create(std::string pluginName, std::string libName);

        void destroy(UserActionPlugin* plugin);

    private:

        void* getHandle(std::string pluginName);

    private:

        std::map<UserActionPlugin*, void*> pluginHandles;
};

}

#endif
