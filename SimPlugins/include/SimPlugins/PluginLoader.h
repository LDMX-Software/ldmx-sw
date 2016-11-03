#ifndef SIMPLUGINS_PLUGINLOADER_H_
#define SIMPLUGINS_PLUGINLOADER_H_

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
