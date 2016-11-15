#ifndef SIMPLUGINS_PLUGINMANAGER_H_
#define SIMPLUGINS_PLUGINMANAGER_H_

// LDMX
#include "SimPlugins/PluginLoader.h"
#include "SimPlugins/UserActionPlugin.h"

// STL
#include <algorithm>
#include <ostream>

// Geant4
#include "G4ClassificationOfNewTrack.hh"

namespace sim {

class PluginManager {

    public:

        typedef std::vector<UserActionPlugin*> PluginVec;

        PluginManager() {;}

        virtual ~PluginManager() {;}

        void beginRun(const G4Run* run) {
            for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
                if ((*it)->hasRunAction()) {
                    (*it)->beginRun(run);
                }
            }
        }

        void endRun(const G4Run* run) {
            for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
                if ((*it)->hasRunAction()) {
                    (*it)->endRun(run);
                }
            }
        }

        void stepping(const G4Step* step) {
            for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
                if ((*it)->hasSteppingAction()) {
                    (*it)->stepping(step);
                }
            }
        }

        void preTracking(const G4Track* track) {
            for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
                if ((*it)->hasTrackingAction()) {
                    (*it)->preTracking(track);
                }
            }
        }

        void postTracking(const G4Track* track) {
            for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
                if ((*it)->hasTrackingAction()) {
                    (*it)->postTracking(track);
                }
            }
        }

        void beginEvent(const G4Event* event) {
            for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
                if ((*it)->hasEventAction()) {
                    (*it)->beginEvent(event);
                }
            }
        }

        void endEvent(const G4Event* event) {
            for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
                if ((*it)->hasEventAction()) {
                    (*it)->endEvent(event);
                }
            }
        }

        /**
         * Return a track classification from the user plugins.
         * The last plugin which sets the classification will override all the others.
         */
        G4ClassificationOfNewTrack stackingClassifyNewTrack(const G4Track* track) {
            G4ClassificationOfNewTrack trackClass = G4ClassificationOfNewTrack::fUrgent;
            for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
                if ((*it)->hasStackingAction()) {
                    trackClass = (*it)->stackingClassifyNewTrack(track);
                }
            }
            return trackClass;
        }

        void stackingNewStage() {
            for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
                if ((*it)->hasEventAction()) {
                    (*it)->stackingNewStage();
                }
            }
        }

        void stackingPrepareNewEvent() {
            for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
                if ((*it)->hasStackingAction()) {
                    (*it)->stackingPrepareNewEvent();
                }
            }
        }

        UserActionPlugin* findPlugin(const std::string& pluginName) {
            UserActionPlugin* foundPlugin = nullptr;
            for (PluginVec::iterator iPlugin = plugins_.begin();
                    iPlugin != plugins_.end(); iPlugin++) {
                if ((*iPlugin)->getName() == pluginName) {
                    foundPlugin = *iPlugin;
                    break;
                }
            }
            return foundPlugin;
        }

        void create(const std::string& pluginName, const std::string& libName) {
            if (findPlugin(pluginName) == nullptr) {
                UserActionPlugin* plugin = pluginLoader_.create(pluginName, libName);
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
                pluginLoader_.destroy(plugin);
            } else {
                std::cerr << "PluginManager::destroy - Plugin "
                        << pluginName << " does not exist." << std::endl;
            }
        }

        std::ostream& print(std::ostream& os) {
            for (PluginVec::iterator iPlugin = plugins_.begin();
                    iPlugin != plugins_.end(); iPlugin++) {
                os << (*iPlugin)->getName() << std::endl;
            }
            return os;
        }

    private:

        void registerPlugin(UserActionPlugin* plugin) {
            plugins_.push_back(plugin);
        }

        void deregisterPlugin(UserActionPlugin* plugin) {
            std::vector<UserActionPlugin*>::iterator pos =
                    std::find(plugins_.begin(), plugins_.end(), plugin);
            if (pos != plugins_.end()) {
                plugins_.erase(pos);
            }
        }

    private:

        PluginLoader pluginLoader_;
        PluginVec plugins_;
};

}

#endif
