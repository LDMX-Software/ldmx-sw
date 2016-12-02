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

        virtual ~PluginManager();

        void beginRun(const G4Run* run);

        void endRun(const G4Run* run);

        void stepping(const G4Step* step);

        void preTracking(const G4Track* track);

        void postTracking(const G4Track* track);

        void beginEvent(const G4Event* event);

        void endEvent(const G4Event* event);

        void generatePrimary(G4Event*);

        /**
         * Return a track classification from the user plugins.
         * The last plugin which sets the classification will override all the others.
         */
        G4ClassificationOfNewTrack stackingClassifyNewTrack(const G4Track* track);

        void stackingNewStage();

        void stackingPrepareNewEvent();

        UserActionPlugin* findPlugin(const std::string& pluginName);

        void create(const std::string& pluginName, const std::string& libName);

        void destroy(const std::string& pluginName);

        std::ostream& print(std::ostream& os);

    private:

        void destroy(UserActionPlugin* plugin);

        void registerPlugin(UserActionPlugin* plugin);

        void deregisterPlugin(UserActionPlugin* plugin);

        /**
         * Destroy all registered plugins.
         */
        void destroyPlugins();

    private:

        PluginLoader pluginLoader_;
        PluginVec plugins_;
};

}

#endif
