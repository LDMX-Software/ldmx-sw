#include "SimPlugins/PluginLoader.h"

using sim::PluginLoader;

int main(int argc, const char* argv[])  {

    PluginLoader* pluginLoader = new PluginLoader();
    sim::UserActionPlugin* plugin = pluginLoader->create("DummySimPlugin", "libSimPlugins.so");
    std::cout << "created plugin: " << plugin << std::endl;
}
