#include "SimPlugins/PluginLoader.h"

using sim::PluginLoader;

int main(int, const char**)  {

    PluginLoader* pluginLoader = new PluginLoader();
    sim::UserActionPlugin* plugin = pluginLoader->create("DummySimPlugin", "libSimPlugins.so");
    std::cout << "created plugin: " << plugin << std::endl;

    pluginLoader->destroy(plugin);

    std::cout << "destroyed plugin" << std::endl;
}
