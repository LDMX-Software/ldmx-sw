#ifndef SimApplication_RootPersistencyManager_h
#define SimApplication_RootPersistencyManager_h

// Geant4
#include "G4PersistencyManager.hh"
#include "G4PersistencyCenter.hh"
#include "G4Run.hh"

// LDMX
#include "Event/RootEventWriter.h"
#include "SimApplication/SimParticleBuilder.h"

using event::RootEventWriter;
using event::Event;

namespace sim {

class RootPersistencyManager : public G4PersistencyManager {

    public:

        RootPersistencyManager()
            : G4PersistencyManager(G4PersistencyCenter::GetPersistencyCenter(), "RootPersistencyManager") {
            G4PersistencyCenter::GetPersistencyCenter()->RegisterPersistencyManager(this);
            G4PersistencyCenter::GetPersistencyCenter()->SetPersistencyManager(this, "RootPersistencyManager");
            this->writer = RootEventWriter::getInstance();
        }

        G4bool Store(const G4Event* anEvent);

        G4bool Store(const G4Run* aRun) {
            RootEventWriter::getInstance()->close();
            return true;
        }

        // Is this ever called???
        void Initialize() {
            std::cout << "RootPersistencyManager::Initialize" << std::endl;
        }

        void openWriter() {
            writer->open();
        }

        Event* getCurrentEvent() {
            return writer->getEvent();
        }

        void clearCurrentEvent() {
            writer->getEvent()->Clear("");
        }

        static RootPersistencyManager* getInstance() {
            return (RootPersistencyManager*) G4PersistencyCenter::GetPersistencyCenter()
                ->GetPersistencyManager("RootPersistencyManager");
        }

    private:

        void writeHeader(const G4Event* anEvent, Event* outputEvent);

        void writeHitsCollections(const G4Event* anEvent, Event* outputEvent);

    private:

        SimParticleBuilder simParticleBuilder;
        RootEventWriter* writer;

};

}

#endif
