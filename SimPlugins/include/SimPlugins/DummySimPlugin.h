#ifndef SimPlugins_DummySimPlugin_h
#define SimPlugins_DummySimPlugin_h

// LDMX
#include "SimPlugins/UserActionPlugin.h"

namespace sim {

class DummySimPlugin : public UserActionPlugin {

    public:

        DummySimPlugin() {
            std::cout << "DummySimPlugin::DummySimPlugin" << std::endl;
        }

        virtual ~DummySimPlugin() {
            std::cout << "DummySimPlugin::~DummySimPlugin" << std::endl;
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

        void beginOfRunAction(G4Run* run) {
            std::cout << "DummySimPlugin::beginOfRunAction - " << run->GetRunID() << std::endl;
        }

        void endOfRunAction(G4Run* run) {
            std::cout << "DummySimPlugin::endOfRunAction - " << run->GetRunID() << std::endl;
        }

        void steppingAction(G4Step* step) {
            std::cout << "DummySimPlugin::steppingAction" << std::endl;
        }

        void preTrackingAction(G4Track* track) {
            std::cout << "DummySimPlugin::preTrackingAction - " << track->GetTrackID() << std::endl;
        }

        void postTrackingAction(G4Track* track) {
            std::cout << "DummySimPlugin::postTrackingAction - " << track->GetTrackID() << std::endl;
        }

        void beginOfEventAction(G4Event* event) {
            std::cout << "DummySimPlugin::beginOfEventAction - " << event->GetEventID() << std::endl;
        }

        void endOfEventAction(G4Event* event) {
            std::cout << "DummySimPlugin::endOfEventAction - " << event->GetEventID() << std::endl;
        }
};

}

#endif
