#ifndef SimPlugins_PluginManager_h
#define SimPlugins_PluginManager_h

#include "SimPlugins/UserActionPlugin.h"

#include <algorithm>

namespace sim {

class PluginManager {

    public:

        typedef std::vector<UserActionPlugin*> PluginVec;

        static PluginManager& getInstance() {
            static PluginManager instance;
            return instance;
        }

        void beginOfRunAction(G4Run* run) {
            for (PluginVec::iterator it = plugins.begin(); it != plugins.end(); it++) {
                if ((*it)->hasRunAction()) {
                    (*it)->beginOfRunAction(run);
                }
            }
        }

        void endOfRunAction(G4Run* run) {
            for (PluginVec::iterator it = plugins.begin(); it != plugins.end(); it++) {
                if ((*it)->hasRunAction()) {
                    (*it)->endOfRunAction(run);
                }
            }
        }

        void steppingAction(G4Step* step) {
            for (PluginVec::iterator it = plugins.begin(); it != plugins.end(); it++) {
                if ((*it)->hasSteppingAction()) {
                    (*it)->steppingAction(step);
                }
            }
        }

        void preTrackingAction(G4Track* track) {
            for (PluginVec::iterator it = plugins.begin(); it != plugins.end(); it++) {
                if ((*it)->hasTrackingAction()) {
                    (*it)->preTrackingAction(track);
                }
            }
        }

        void postTrackingAction(G4Track* track) {
            for (PluginVec::iterator it = plugins.begin(); it != plugins.end(); it++) {
                if ((*it)->hasTrackingAction()) {
                    (*it)->postTrackingAction(track);
                }
            }
        }

        void beginOfEventAction(G4Event* event) {
            for (PluginVec::iterator it = plugins.begin(); it != plugins.end(); it++) {
                if ((*it)->hasEventAction()) {
                    (*it)->beginOfEventAction(event);
                }
            }
        }

        void endOfEventAction(G4Event* event) {
            for (PluginVec::iterator it = plugins.begin(); it != plugins.end(); it++) {
                if ((*it)->hasEventAction()) {
                    (*it)->endOfEventAction(event);
                }
            }
        }

        void addPlugin(UserActionPlugin* plugin) {
            plugins.push_back(plugin);
        }

        void removePlugin(UserActionPlugin* plugin) {
            std::vector<UserActionPlugin*>::iterator pos =
                    std::find(plugins.begin(), plugins.end(), plugin);
            if (pos != plugins.end()) {
                plugins.erase(pos);
            }
        }

    private:

        PluginVec plugins;
};

}

#endif
