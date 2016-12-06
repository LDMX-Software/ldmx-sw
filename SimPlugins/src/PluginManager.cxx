#include "SimPlugins/PluginManager.h"

namespace sim {

PluginManager::~PluginManager() {
    destroyPlugins();
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

void PluginManager::generatePrimary(G4Event* event) {
    for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
        if ((*it)->hasPrimaryGeneratorAction()) {
            (*it)->generatePrimary(event);
        }
    }
}

/**
 * @brief Return a track classification from the user plugins that have stacking actions.
 *
 * @note The current classification will only be updated if a plugin actually provides a different classification
 *       than the current one.  By default, plugins that do not wish to change a track classification will return
 *       the value of <i>currentTrackClass</i> in order to not override values from previously activated plugins.
 */
G4ClassificationOfNewTrack PluginManager::stackingClassifyNewTrack(const G4Track* track) {

    // Default value of a track is fUrgent.
    G4ClassificationOfNewTrack currentTrackClass = G4ClassificationOfNewTrack::fUrgent;

    for (PluginVec::iterator it = plugins_.begin(); it != plugins_.end(); it++) {
        if ((*it)->hasStackingAction()) {

            // Get proposed new track classification from this plugin.
            G4ClassificationOfNewTrack newTrackClass = (*it)->stackingClassifyNewTrack(track, currentTrackClass);

            // Only set the current classification if the plugin changed it.
            if (newTrackClass != currentTrackClass) {

                // Set the track classification from this plugin.
                currentTrackClass = newTrackClass;
            }
        }
    }

    // Return the current track classification.
    return currentTrackClass;
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
        std::cerr << "[ PluginManager ] - Plugin " << pluginName << " already exists." << std::endl;
        throw new std::runtime_error("Plugin already loaded.");
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

void PluginManager::destroyPlugins() {
    for (int iPlugin = 0; iPlugin < plugins_.size(); iPlugin++) {
        destroy(plugins_[iPlugin]);
    }
    plugins_.clear();
}

} // namespace sim
