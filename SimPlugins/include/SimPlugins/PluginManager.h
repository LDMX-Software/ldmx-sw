/**
 * @file PluginManager.h
 * @brief Class for managing the loading, activation and destruction of UserSimPlugin objects
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

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

namespace ldmx {

/**
 * @class PluginManager
 * @brief Manages user sim plugins
 *
 * @note
 * This class loads UserSimPlugin objects from dynamic libraries using a PluginLoader.
 * It is also responsible for activating the user action hooks for all registered plugins.
 * Only one instance of a given plugin can be loaded at a time.
 *
 * @see UserActionPlugin
 * @see PluginLoader
 */
class PluginManager {

    public:

        /**
         * Vector of plugins.
         */
        typedef std::vector<UserActionPlugin*> PluginVec;

        /**
         * Class constructor.
         */
        PluginManager() {;}

        /**
         * Class destructor.
         * This will destroy all the currently registered plugins.
         */
        virtual ~PluginManager();

        /**
         * Activate the begin run hook for registered plugins.
         * @param aRun The Geant4 run that is beginning.
         */
        void beginRun(const G4Run* aRun);

        /**
         * Activate the end run hook of registered plugins.
         * @param aRun The Geant4 run that is ending.
         */
        void endRun(const G4Run* aRun);

        /**
         * Activate the stepping hook of registered plugins.
         * @param aStep The Geant4 step.
         */
        void stepping(const G4Step* aStep);

        /**
         * Activate the pre-tracking hook of registered plugins.
         * @param aTrack The Geant4 track.
         */
        void preTracking(const G4Track* aTrack);

        /**
         * Activate the post-tracking hook of registered plugins.
         * @param aTrack The Geant4 track.
         */
        void postTracking(const G4Track* aTrack);

        /**
         * Activate the begin event hook of registered plugins.
         * @param anEvent The Geant4 event.
         */
        void beginEvent(const G4Event* anEvent);

        /**
         * Activate the end event hook of registered plugins.
         * @param anEvent The Geant4 event.
         */
        void endEvent(const G4Event* anEvent);

        /**
         * Activate the generate primary hook of registered plugins.
         * @param anEvent The Geant4 event.
         */
        void generatePrimary(G4Event* anEvent);

        /**
         *
         * Activate the track classification hook of registered plugins.
         * @param aTrack The Geant4 track.
         *
         * @brief Return a track classification from the user plugins that have stacking actions.
         *
         * @note
         * The current classification will only be updated if a plugin actually provides a different classification
         * than the current one.
         */
        G4ClassificationOfNewTrack stackingClassifyNewTrack(const G4Track* aTrack);

        /**
         * Activate the stacking new stage hook of registered plugins.
         */
        void stackingNewStage();

        /**
         * Activate the prepare new event stacking hook of registered plugins.
         */
        void stackingPrepareNewEvent();

        /**
         * Find a plugin by name.
         * @param pluginName The name of the plugin.
         * @return The plugin with the name or <i>nullptr</i> if does not exist.
         */
        UserActionPlugin* findPlugin(const std::string& pluginName);

        /**
         * Create a new plugin by name from the given library.
         * @param pluginName The name of the plugin.
         * @param libName The name of the shared library.
         */
        void create(const std::string& pluginName, const std::string& libName);

        /**
         * Destroy a plugin by name.
         * @param pluginName The name of the plugin.
         */
        void destroy(const std::string& pluginName);

        /**
         * Print out information about registered plugins.
         * @param os The output stream.
         * @return The same output stream.
         */
        std::ostream& print(std::ostream& os);

    private:

        /**
         * Destroy a plugin.
         * @param plugin Pointer to plugin that should be destroyed.
         */
        void destroy(UserActionPlugin* plugin);

        /**
         * Register a plugin.
         * @param plugin Pointer to plugin that should be registered.
         */
        void registerPlugin(UserActionPlugin* plugin);

        /**
         * De-register a plugin.
         * @param plugin Pointer to plugin that should be de-registered.
         */
        void deregisterPlugin(UserActionPlugin* plugin);

        /**
         * Destroy all registered plugins.
         */
        void destroyPlugins();

    private:

        /**
         * The plugin loader for loading plugins from shared libraries.
         */
        PluginLoader pluginLoader_;

        /**
         * The list of registered plugins.
         */
        PluginVec plugins_;
};

}

#endif
