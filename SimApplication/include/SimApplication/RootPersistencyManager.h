/**
 * @file RootPersistencyManager.h
 * @brief Class providing persistency manager implementation with SimEvent output
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_ROOTPERSISTENCYMANAGER_H_
#define SIMAPPLICATION_ROOTPERSISTENCYMANAGER_H_

// Geant4
#include "G4PersistencyManager.hh"
#include "G4PersistencyCenter.hh"
#include "G4Run.hh"

// LDMX
#include "Event/RootEventWriter.h"
#include "Event/SimEvent.h"
#include "SimApplication/EcalHitIO.h"
#include "SimApplication/G4TrackerHit.h"
#include "SimApplication/SimParticleBuilder.h"

using detdescr::DetectorID;
using detdescr::EcalDetectorID;
using event::RootEventWriter;
using event::Event;
using event::SimEvent;

namespace sim {

/**
 * @class RootPersistencyManager
 * @brief Provides a <i>G4PersistencyManager</i> implemention with SimEvent output
 *
 * @note
 * Output is written at the end of each event.  A RootEventWriter is used to write
 * from an Event buffer into an output branch within a tree.  The Event buffer is cleared
 * after the event is written.  A SimParticleBuilder is used to build a set of SimParticle
 * objects from the Trajectory objects which were created during event processing.
 * An EcalHitIO instance provides translation of G4CalorimeterHit objects to an output
 * SimCalorimeterHit collection.  The tracker hit collections of G4TrackerHit objects
 * are translated directly into SimTrackerHit collections.
 */
class RootPersistencyManager : public G4PersistencyManager {

    public:

        /**
         * Class constructor.
         * Installs the object as the global persistency manager.
         */
        RootPersistencyManager();

        /**
         * Get the current ROOT persistency manager or <i>nullptr</i> if not registered.
         * @return The ROOT persistency manager.
         */
        static RootPersistencyManager* getInstance() {
            return (RootPersistencyManager*) G4PersistencyCenter::GetPersistencyCenter()->CurrentPersistencyManager();
        }

        /**
         * Builds the output ROOT event.
         * @param anEvent The Geant4 event data.
         */
        G4bool Store(const G4Event* anEvent);

        /**
         * This gets called automatically at the end of the run and is used to close the writer.
         * @param aRun The Geant4 run data.
         * @return True if event is stored (hard-coded to true).
         */
        G4bool Store(const G4Run* aRun) {
            if (m_verbose > 1) {
                std::cout << "RootPersistencyManager::Store - closing writer for run " << aRun->GetRunID() << std::endl;
            }
            writer_->close();
            return true;
        }

        /**
         * Implementing this makes an "overloaded-virtual" compiler warning go away.
         */
        G4bool Store(const G4VPhysicalVolume*) { return false; }

        /** 
          * This is called "manually" in UserRunAction to open the ROOT writer for the run.
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

        /**
         * Get the current event from the ROOT writer.
         * @return The current event.
         */
        Event* getCurrentEvent() {
            return writer_->getEvent();
        }

        /**
         * Set the output file name.
         * @param fileName The output file name.
         */
        void setFileName(std::string fileName) {
            this->writer_->setFileName(fileName);
        }
        
        /**
         * Get the ROOT writer.
         * @return The ROOT writer.
         */
        RootEventWriter* getWriter() {
            return writer_;
        }

        /**
         * Enable or disable hit contribution output for SimCalorimeterHits.
         * This is enabled by default.
         * @param enableHitContribs True to enable hit contributions.
         */
        void setEnableHitContribs(bool enableHitContribs) {
            // Pass this flag to the ECal IO helper.
            ecalHitIO_->setEnableHitContribs(enableHitContribs);
        }

        /**
         * Enable or disable compression of hit contribution output by finding
         * matching SimParticle and PDG codes and updating the existing record.
         * This is enabled by default.
         * @param compressHitContribs True to compress hit contributions.
         */
        void setCompressHitContribs(bool compressHitContribs) {
            // Pass this flag to the ECal IO helper.
            ecalHitIO_->setCompressHitContribs(compressHitContribs);
        }

    private:
        
        /**
         * Build an output event from the current Geant4 event.
         * @param anEvent The Geant4 event.
         * @param outputEvent The output event.
         */
        void buildEvent(const G4Event* anEvent, Event* outputEvent);

        /**
         * Write header info into the output event from Geant4.
         * @param anEvent The Geant4 event.
         * @param outputEvent The output event.
         */
        void writeHeader(const G4Event* anEvent, Event* outputEvent);

        /**
         * Write hits collections from Geant4 into a ROOT event.
         * @param anEvent The Geant4 event.
         * @param outputEvent The output event.
         */
        void writeHitsCollections(const G4Event* anEvent, Event* outputEvent);
        
        /**
         * Write a collection of tracker hits to an output collection.
         * @param hc The collection of G4TrackerHits.
         * @param outputColl The output collection of SimTrackerHits.
         */
        void writeTrackerHitsCollection(G4TrackerHitsCollection* hc, TClonesArray* outputColl);

        /**
         * Write a collection of tracker hits to an output collection.
         * @param hc The collection of G4CalorimeterHits.
         * @param outputColl The output collection of SimCalorimeterHits.
         */
        void writeCalorimeterHitsCollection(G4CalorimeterHitsCollection* hc, TClonesArray* outputColl);

        /**
         * Print out event info and data depending on the verbose level.
         * @param anEvent The output event.
         */
        void printEvent(Event* anEvent);

    private:

        /**
         * Helper for building output SimParticle collection.
         */
        SimParticleBuilder simParticleBuilder_;

        /**
         * Writes out events to ROOT file.
         */
        RootEventWriter* writer_;

        /**
         * Handles ECal hit readout and IO.
         */
        EcalHitIO* ecalHitIO_;
    };

}

#endif
