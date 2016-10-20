#ifndef SimPlugins_DummySimPlugin_h
#define SimPlugins_DummySimPlugin_h

// LDMX
#include "SimPlugins/UserActionPlugin.h"

namespace sim {

class DummySimPlugin : public UserActionPlugin {

    public:

        DummySimPlugin() {
            std::cout << "DummySimPlugin::DummySimPlugin - Hello!" << std::endl;
        }

        virtual ~DummySimPlugin() {
            std::cout << "DummySimPlugin::~DummySimPlugin - Goodbye!" << std::endl;
        }

        virtual std::string getName() {
            return "DummySimPlugin";
        }

        bool hasRunAction() {
            return true;
        }

        bool hasSteppingAction() {
            return true;
        }

        bool hasTrackingAction() {
            return true;
        }

        bool hasEventAction() {
            return true;
        }

        void beginRun(const G4Run* run) {
            std::cout << "DummySimPlugin::beginRun - " << run->GetRunID() << std::endl;
        }

        void endRun(const G4Run* run) {
            std::cout << "DummySimPlugin::endRun - " << run->GetRunID() << std::endl;
        }

        void stepping(const G4Step* step) {
            std::cout << "DummySimPlugin::stepping" << std::endl;
        }

        void preTracking(const G4Track* track) {
            std::cout << "DummySimPlugin::preTracking - " << track->GetTrackID() << std::endl;
        }

        void postTracking(const G4Track* track) {
            std::cout << "DummySimPlugin::postTracking - " << track->GetTrackID() << std::endl;
        }

        void beginEvent(const G4Event* event) {
            std::cout << "DummySimPlugin::beginEvent - " << event->GetEventID() << std::endl;
        }

        void endEvent(const G4Event* event) {
            std::cout << "DummySimPlugin::endEvent - " << event->GetEventID() << std::endl;
        }
};

}

#endif
