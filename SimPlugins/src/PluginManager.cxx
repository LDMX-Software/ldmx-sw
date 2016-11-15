#include "SimPlugins/PluginManager.h"

namespace sim {

PluginManager::~PluginManager() {
    for (PluginVec::iterator iPlugin = plugins_.begin();
            iPlugin != plugins_.end();
            iPlugin++) {
        destroy((*iPlugin)->getName());
    }
}

void PluginManager::beginRun(const G4Run* run) {
    for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
        if ((*it)->hasRunAction()) {
            (*it)->beginRun(run);
        }
    }
}

void PluginManager::endRun(const G4Run* run) {
    for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
        if ((*it)->hasRunAction()) {
            (*it)->endRun(run);
        }
    }
}

void PluginManager::stepping(const G4Step* step) {
    for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
        if ((*it)->hasSteppingAction()) {
            (*it)->stepping(step);
        }
    }
}

void PluginManager::preTracking(const G4Track* track) {
    for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
        if ((*it)->hasTrackingAction()) {
            (*it)->preTracking(track);
        }
    }
}

void PluginManager::postTracking(const G4Track* track) {
    for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
        if ((*it)->hasTrackingAction()) {
            (*it)->postTracking(track);
        }
    }
}

void PluginManager::beginEvent(const G4Event* event) {
    for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
        if ((*it)->hasEventAction()) {
            (*it)->beginEvent(event);
        }
    }
}

void PluginManager::endEvent(const G4Event* event) {
    for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
        if ((*it)->hasEventAction()) {
            (*it)->endEvent(event);
        }
    }
}

/**
 * @brief Return a track classification from the user plugins that have stacking actions.
 *
 * @note The setting from the last plugin in the list will override all the others.
 * For this reason, the user should likely only activate one plugin at a time that
 * implements the stacking action hooks.
 */
G4ClassificationOfNewTrack PluginManager::stackingClassifyNewTrack(const G4Track* track) {
    G4ClassificationOfNewTrack trackClass = G4ClassificationOfNewTrack::fUrgent;
    for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
        if ((*it)->hasStackingAction()) {
            G4ClassificationOfNewTrack trackClass = (*it)->stackingClassifyNewTrack(track);
        }
    }
    return trackClass;
}

void PluginManager::stackingNewStage() {
    for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
        if ((*it)->hasEventAction()) {
            (*it)->stackingNewStage();
        }
    }
}

void PluginManager::stackingPrepareNewEvent() {
    for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
        if ((*it)->hasStackingAction()) {
            (*it)->stackingPrepareNewEvent();
        }
    }
}

UserActionPlugin* PluginManager::findPlugin(const std::string& pluginName) {
    UserActionPlugin* foundPlugin = nullptr;
    for (PluginVec::iterator iPlugin = plugins_.begin(); iPlugin != plugins_.end(); iPlugin++) {
        if ((*iPlugin)->getName() == pluginName) {
            foundPlugin = *iPlugin;
            break;
        }
    }
    return foundPlugin;
}

void PluginManager::create(const std::string& pluginName, const std::string& libName) {
    if (findPlugin(pluginName) == nullptr) {
        UserActionPlugin* plugin = pluginLoader_.create(pluginName, libName);
        registerPlugin(plugin);
    } else {
        std::cerr << "PluginManager::create - Plugin " << pluginName << " already exists." << std::endl;
    }
}

void PluginManager::destroy(const std::string& pluginName) {
    UserActionPlugin* plugin = findPlugin(pluginName);
    if (plugin != nullptr) {
        destroy(plugin);
    }
}

void PluginManager::destroy(UserActionPlugin* plugin) {
    if (plugin != nullptr) {
        deregisterPlugin(plugin);
        pluginLoader_.destroy(plugin);
    }
}

std::ostream& PluginManager::print(std::ostream& os) {
    for (PluginVec::iterator iPlugin = plugins_.begin(); iPlugin != plugins_.end(); iPlugin++) {
        os << (*iPlugin)->getName() << std::endl;
    }
    return os;
}

void PluginManager::registerPlugin(UserActionPlugin* plugin) {
    plugins_.push_back(plugin);
}

void PluginManager::deregisterPlugin(UserActionPlugin* plugin) {
    std::vector<UserActionPlugin*>::iterator pos =
            std::find(plugins_.begin(), plugins_.end(), plugin);
    if (pos != plugins_.end()) {
        plugins_.erase(pos);
    }
}

} // namespace sim
