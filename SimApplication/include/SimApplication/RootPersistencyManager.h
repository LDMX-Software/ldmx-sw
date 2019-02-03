/**
 * @file RootPersistencyManager.h
 * @brief Class used to manage ROOT based persistency.
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _SIMAPPLICATION_ROOTPERSISTENCYMANAGER_H_
#define _SIMAPPLICATION_ROOTPERSISTENCYMANAGER_H_

//------------//
//   Geant4   //
//------------//
#include "G4PersistencyManager.hh"
#include "G4PersistencyCenter.hh"

//-------------//
//   ldmx-sw   //
//-------------//
#include "SimApplication/EcalHitIO.h"
#include "SimApplication/G4CalorimeterHit.h"
#include "SimApplication/G4TrackerHit.h"
#include "SimApplication/SimParticleBuilder.h"

// Forward declarations
class G4Run; 

namespace ldmx {

    // Forward declarations within the ldmx namespace
    class EcalHitIO; 
    class Event;
    class EventFile;
    class RunHeader;
    
    /**
     * @class RootPersistencyManager
     *
     * @note
     * Output is written at the end of each event.  An EventFile is used to 
     * write from an Event buffer object into an output branch within a tree.
     * The event buffer is cleared after the event is written.  A 
     * SimParticleBuilder is used to build a set of SimParticle objects from 
     * the Trajectory objects which were created during event processing. An 
     * EcalHitIO instance provides translation of G4CalorimeterHit objects in
     * the ECal to an output SimCalorimeterHit collection, transforming the 
     * individual steps into cell energy depositions.  The tracker hit
     * collections of G4TrackerHit objects are translated directly into 
     * output SimTrackerHit collections.
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
                return static_cast<RootPersistencyManager*>(
                        G4PersistencyCenter::GetPersistencyCenter()->CurrentPersistencyManager());
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
            G4bool Store(const G4VPhysicalVolume*) {
                return false;
            }

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

            /** 
             * Drop the hits associated with the specified collection.
             *
             * @param collectionName The name of the collection whose hits
             *                       should be dropped.
             */
            void dropCollection(std::string collectionName) { 
                dropCollectionNames_.push_back(collectionName);  
            }  

            /** 
             * Set the description of the run. 
             *
             * @param description Description of the run.
             */
            void setRunDescription(std::string description) { description_ = description; }

            /**
             * Set the run number. 
             *
             * @param runNumber The run number.
             */
            void setRunNumber(int runNumber) { runNumber_ = runNumber; } 

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
             * Write header info into the output event from Geant4.
             * @param fileName The filename that stores temporary seeds.
             */
            std::string getEventSeeds(std::string fileName = "currentEvent.rndm");

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
            RunHeader* createRunHeader(const G4Run* aRun);

            /**
             * Create the run header and write it into the current output file.
             * @param aRun The current Geant4 run.
             */
            void writeRunHeader(const G4Run* aRun);

        private:

            /** List of collections whose hits should be droppped. */
            std::vector<std::string> dropCollectionNames_; 

            /**
             * The output file name.
             */
            std::string fileName_ {"ldmx_sim_events.root"};

            /** Description of this run. */
            std::string description_{"LDMX simulated events."}; 

            /** Run number. */
            int runNumber_{0}; 

            /**
             * The output file with the event tree.
             */
            EventFile* outputFile_ {nullptr};

            /**
             * Output file compression level.
             */
            int compressionLevel_ {6};

            /**
             * The event container used to manage the tree/branches/collections.
             */
            Event* event_ {nullptr};

            /**
             * Event header for writing out event number, etc. into output.
             */
            EventHeader eventHeader_;

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
