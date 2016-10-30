#ifndef SIMPLUGINS_PLUGINMANAGER_H_
#define SIMPLUGINS_PLUGINMANAGER_H_

// LDMX
#include "SimPlugins/PluginLoader.h"
#include "SimPlugins/UserActionPlugin.h"

// STL
#include <algorithm>
#include <ostream>

namespace sim {

class PluginManager {

    public:

        typedef std::vector<UserActionPlugin*> PluginVec;

        static PluginManager& getInstance() {
            static PluginManager instance;
            return instance;
        }

        void beginRun(const G4Run* run) {
            for (PluginVec::iterator it = plugins.begin(); it != plugins.end(); it++) {
                if ((*it)->hasRunAction()) {
                    (*it)->beginRun(run);
                }
            }
        }

        void endRun(const G4Run* run) {
            for (PluginVec::iterator it = plugins.begin(); it != plugins.end(); it++) {
                if ((*it)->hasRunAction()) {
                    (*it)->endRun(run);
                }
            }
        }

        void stepping(const G4Step* step) {
            for (PluginVec::iterator it = plugins.begin(); it != plugins.end(); it++) {
                if ((*it)->hasSteppingAction()) {
                    (*it)->stepping(step);
                }
            }
        }

        void preTracking(const G4Track* track) {
            for (PluginVec::iterator it = plugins.begin(); it != plugins.end(); it++) {
                if ((*it)->hasTrackingAction()) {
                    (*it)->preTracking(track);
                }
            }
        }

        void postTracking(const G4Track* track) {
            for (PluginVec::iterator it = plugins.begin(); it != plugins.end(); it++) {
                if ((*it)->hasTrackingAction()) {
                    (*it)->postTracking(track);
                }
            }
        }

        void beginEvent(const G4Event* event) {
            for (PluginVec::iterator it = plugins.begin(); it != plugins.end(); it++) {
                if ((*it)->hasEventAction()) {
                    (*it)->beginEvent(event);
                }
            }
        }

        void endEvent(const G4Event* event) {
            for (PluginVec::iterator it = plugins.begin(); it != plugins.end(); it++) {
                if ((*it)->hasEventAction()) {
                    (*it)->endEvent(event);
                }
            }
        }

        UserActionPlugin* findPlugin(const std::string& pluginName) {
            UserActionPlugin* foundPlugin = nullptr;
            for (PluginVec::iterator iPlugin = plugins.begin();
                    iPlugin != plugins.end(); iPlugin++) {
                if ((*iPlugin)->getName() == pluginName) {
                    foundPlugin = *iPlugin;
                    break;
                }
            }
            return foundPlugin;
        }

        void create(const std::string& pluginName, const std::string& libName) {
            if (findPlugin(pluginName) == nullptr) {
                UserActionPlugin* plugin = pluginLoader.create(pluginName, libName);
                registerPlugin(plugin);
            } else {
                std::cerr << "PluginManager::create - Plugin " << pluginName
                        << " already exists." << std::endl;
            }
        }

        void destroy(const std::string& pluginName) {
            UserActionPlugin* plugin = findPlugin(pluginName);
            if (plugin != nullptr) {
                deregisterPlugin(plugin);
                pluginLoader.destroy(plugin);
            } else {
                std::cerr << "PluginManager::destroy - Plugin "
                        << pluginName << " does not exist." << std::endl;
            }
        }

        std::ostream& print(std::ostream& os) {
            for (PluginVec::iterator iPlugin = plugins.begin();
                    iPlugin != plugins.end(); iPlugin++) {
                os << (*iPlugin)->getName() << std::endl;
            }
            return os;
        }

    private:

        void registerPlugin(UserActionPlugin* plugin) {
            plugins.push_back(plugin);
        }

        void deregisterPlugin(UserActionPlugin* plugin) {
            std::vector<UserActionPlugin*>::iterator pos =
                    std::find(plugins.begin(), plugins.end(), plugin);
            if (pos != plugins.end()) {
                plugins.erase(pos);
            }
        }

    private:

        PluginLoader pluginLoader;
        PluginVec plugins;
};

}

#endif
