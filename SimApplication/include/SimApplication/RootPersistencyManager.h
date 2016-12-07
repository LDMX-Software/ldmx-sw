/**
 * @file RootPersistencyManager.h
 * @brief Persistency manager implementation providing sim event output from the Geant4 session.
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_ROOTPERSISTENCYMANAGER_H_
#define SIMAPPLICATION_ROOTPERSISTENCYMANAGER_H_

// Geant4
#include "G4PersistencyManager.hh"
#include "G4PersistencyCenter.hh"
#include "G4Run.hh"

// LDMX
#include "DetDescr/DefaultDetectorID.h"
#include "DetDescr/DetectorID.h"
#include "Event/SimEvent.h"
#include "Event/RootEventWriter.h"
#include "SimApplication/SimParticleBuilder.h"

using detdescr::DetectorID;
using detdescr::DefaultDetectorID;
using event::RootEventWriter;
using event::Event;
using event::SimEvent;

namespace sim {

class RootPersistencyManager : public G4PersistencyManager {

    public:
		typedef std::pair<int, int> layer_cell_pair;

        /** Constructor, which will install the object as the global persistency manager. */
        RootPersistencyManager();

        /** This is the primary method for building the output ROOT event. */
        G4bool Store(const G4Event* anEvent);

        /** This gets called automatically at the end of the run and is used to close the writer. */
        G4bool Store(const G4Run* aRun) {
            if (m_verbose > 1) {
                std::cout << "RootPersistencyManager::Store - closing writer for run " << aRun->GetRunID() << std::endl;
            }
            writer_->close();
            return true;
        }

        /** Implementing this makes an "overloaded-virtual" compiler warning go away. */
        G4bool Store(const G4VPhysicalVolume*) { return false; }

        /** 
          * This is called "manually" in UserRunAction to open the ROOT writer for the run.
          * Is Geant4 supposed to activate this someplace?
          */
        void Initialize() {
            if (m_verbose > 1) {
                std::cout << "RootPersistencyManager: Opening " << writer_->getFileName() << " with mode " 
                    << writer_->getMode() << " and compression " << writer_->getCompression() << std::endl;
            }            
     
            writer_->open();
                               
            // If we can't write to the output file then the run must be aborted immediately.
            if (!writer_->getFile()->IsWritable()) {
                G4Exception("RootPersistencyManager::Initialize", "", RunMustBeAborted, "Output ROOT file is not writable.");
            }
        }

        Event* getCurrentEvent() {
            return writer_->getEvent();
        }

        void setFileName(std::string fileName) {
            this->writer_->setFileName(fileName);
        }
        
        RootEventWriter* getWriter() {
            return writer_;
        }

        /** Return the current ROOT persistency manager or null if it is disabled. */
        static RootPersistencyManager* getInstance() {
            return (RootPersistencyManager*) G4PersistencyCenter::GetPersistencyCenter()->CurrentPersistencyManager();
        }

    private:
        
        /** Build an output event from the current Geant4 event. */
        void buildEvent(const G4Event*, Event*);

        /** Write header info into the output event from Geant4. */
        void writeHeader(const G4Event* anEvent, Event* outputEvent);

        /** Write hits collections from Geant4 into a ROOT event. */
        void writeHitsCollections(const G4Event* anEvent, Event* outputEvent);
        
        /** Print out event info and data depending on the verbose level. */
        void printEvent(Event*);
        

    private:

        SimParticleBuilder simParticleBuilder_;
        RootEventWriter* writer_;
        DetectorID* detID;
        std::map<layer_cell_pair, int> ecalReadoutMap;

};

}

#endif
