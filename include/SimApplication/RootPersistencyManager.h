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
#include "Event/EventFile.h"
#include "Event/EventHeader.h"
#include "Event/EventImpl.h"
#include "Event/RunHeader.h"
#include "SimApplication/EcalHitIO.h"
#include "SimApplication/G4TrackerHit.h"
#include "SimApplication/SimParticleBuilder.h"

namespace sim {

/**
 * @class RootPersistencyManager
 * @brief Provides a <i>G4PersistencyManager</i> implemention with SimEvent output
 *
 * @note
 * Output is written at the end of each event.  An EventFile is used to write from an
 * EventImpl buffer object into an output branch within a tree.  The event buffer is cleared
 * after the event is written.  A SimParticleBuilder is used to build a set of SimParticle
 * objects from the Trajectory objects which were created during event processing.
 * An EcalHitIO instance provides translation of G4CalorimeterHit objects in the ECal
 * to an output SimCalorimeterHit collection, transforming the individual steps into
 * cell energy depositions.  The tracker hit collections of G4TrackerHit objects are
 * translated directly into output SimTrackerHit collections.
 */
class RootPersistencyManager : public G4PersistencyManager {

    public:

        typedef std::map<std::string, TClonesArray*> HitsCollectionMap;

        /**
         * Class constructor.
         * Installs the object as the global persistency manager.
         */
        RootPersistencyManager();

        virtual ~RootPersistencyManager() {
            for (auto entry : outputHitsCollections_) {
                delete entry.second;
            }
        }

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
         * This gets called automatically at the end of the run and is used to write out the run header
         * and close the writer.
         * @param aRun The Geant4 run data.
         * @return True if event is stored (function is hard-coded to return true).
         */
        G4bool Store(const G4Run* aRun);

        /**
         * Implementing this makes an "overloaded-virtual" compiler warning go away.
         */
        G4bool Store(const G4VPhysicalVolume*) { return false; }

        /** 
          * This is called "manually" in UserRunAction to open the ROOT writer for the run.
          */
        void Initialize();

        /**
         * Set the output file name.
         * @param fileName The output file name.
         */
        void setFileName(std::string fileName) {
            fileName_ = fileName;
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

        void setCompressionLevel(int compressionLevel) {
            compressionLevel_ = compressionLevel;
        }

    private:
        
        /**
         * Build an output event from the current Geant4 event.
         * @param anEvent The Geant4 event.
         * @param outputEvent The output event.
         */
        void buildEvent(const G4Event* anEvent, event::EventImpl* outputEvent);

        /**
         * Write header info into the output event from Geant4.
         * @param anEvent The Geant4 event.
         * @param outputEvent The output event.
         */
        void writeHeader(const G4Event* anEvent, event::EventImpl* outputEvent);

        /**
         * Write hits collections from Geant4 into a ROOT event.
         * @param anEvent The Geant4 event.
         * @param outputEvent The output event.
         */
        void writeHitsCollections(const G4Event* anEvent, event::EventImpl* outputEvent);
        
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
        void printEvent(event::EventImpl* anEvent);

        /**
         * Setup a map of HC names to output TClonesArray collections.
         * This is called once at the beginning of the run.
         */
        void setupHitsCollectionMap();

        /**
         * Create the run header for writing into the output file.
         * @param aRun The current Geant4 run.
         * @return The created run header.
         */
        event::RunHeader* createRunHeader(const G4Run* aRun);

        /**
         * Create the run header and write it into the current output file.
         * @param aRun The current Geant4 run.
         */
        void writeRunHeader(const G4Run* aRun);

    private:

        /**
         * The output file name.
         */
        std::string fileName_{"ldmx_sim_events.root"};

        /**
         * The output file with the event tree.
         */
        event::EventFile* outputFile_{nullptr};

        /**
         * Output file compression level.
         */
        int compressionLevel_{6};

        /**
         * The event container used to manage the tree/branches/collections.
         */
        event::EventImpl* event_{nullptr};

        /**
         * Event header for writing out event number, etc. into output.
         */
        event::EventHeader eventHeader_;

        /**
         * Handles ECal hit readout and IO.
         */
        EcalHitIO* ecalHitIO_;

        /**
         * Helper for building output SimParticle collection.
         */
        SimParticleBuilder simParticleBuilder_;

        /**
         * Map of HC names to output TClonesArray collection.
         */
        HitsCollectionMap outputHitsCollections_;

    };

}

#endif
