/**
 * @file RootPersistencyManager.cxx
 * @brief Class used to manage ROOT based persistency.
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimApplication/RootPersistencyManager.h"

//----------------//
//   C++ StdLib   //
//----------------//
#include <algorithm>

//-----------//
//   Boost   //
//-----------//
#include "boost/format.hpp"

//-------------//
//   ldmx-sw   //
//-------------//
#include "Event/Event.h"
#include "Event/EventHeader.h"
#include "Event/RunHeader.h"
#include "Framework/EventFile.h"
#include "Framework/EventImpl.h"
#include "Event/EventConstants.h"
#include "SimApplication/CalorimeterSD.h"
#include "SimApplication/DetectorConstruction.h"
#include "SimApplication/RunManager.h"
#include "SimApplication/TrackerSD.h"
#include "SimApplication/ScoringPlaneSD.h"

//------------//
//   Geant4   //
//------------//
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4RunManagerKernel.hh"
#include "G4SDManager.hh"

namespace ldmx {

    RootPersistencyManager::RootPersistencyManager() :
        G4PersistencyManager(G4PersistencyCenter::GetPersistencyCenter(), "RootPersistencyManager"), 
        ecalHitIO_(new EcalHitIO(&simParticleBuilder_)) 
    {
        G4PersistencyCenter::GetPersistencyCenter()->RegisterPersistencyManager(this);
        G4PersistencyCenter::GetPersistencyCenter()->SetPersistencyManager(this, "RootPersistencyManager");

        event_ = new EventImpl("sim");
    }

    G4bool RootPersistencyManager::Store(const G4Event* anEvent) {

        // verbose level 2
        if (m_verbose > 1) {
            std::cout << "[ RootPersistencyManager ] : Storing event " << anEvent->GetEventID() << std::endl;
        }

        if (G4RunManager::GetRunManager()->GetCurrentEvent()->IsAborted()) {
            // TODO: Need event cleanup here?
            return false;
        }

        // Build the output collections.
        buildEvent(anEvent, event_);

        // Print out event info and data depending on verbose level.
        printEvent(event_);

        outputFile_->nextEvent();

        return true;
    }

    void RootPersistencyManager::writeRunHeader(const G4Run* aRun) {
        RunHeader* runHeader = createRunHeader(aRun);
        outputFile_->writeRunHeader(runHeader);
        delete runHeader;
    }

    G4bool RootPersistencyManager::Store(const G4Run* aRun) {
        if (m_verbose > 1) {
            std::cout << "[ RootPersistencyManager ] : Storing run " << aRun->GetRunID() << std::endl;
        }

        // Write out the run header.
        writeRunHeader(aRun);

        // Close the file and delete the output file object.
        outputFile_->close();
        delete outputFile_;
        outputFile_ = nullptr;

        return true;
    }

    void RootPersistencyManager::Initialize() {

        if (m_verbose > 1) {
            std::cout << "[ RootPersistencyManager ] : Opening output file " << fileName_ << std::endl;
        }

        // Create and setup the output file for writing the events.
        outputFile_ = new EventFile(fileName_.c_str(), true, compressionLevel_);
        outputFile_->setupEvent((EventImpl*) event_);

        // Create map with output hits collections.
        setupHitsCollectionMap();
    }

    void RootPersistencyManager::buildEvent(const G4Event* anEvent, Event* outputEvent) {

        // Set basic event information.
        writeHeader(anEvent, outputEvent);

        // Set pointer to current G4Event.
        simParticleBuilder_.setCurrentEvent(anEvent);

        // Build the SimParticle list for the output ROOT event.
        simParticleBuilder_.buildSimParticles(outputEvent);

        // Copy hit objects from SD hit collections into the output event.
        writeHitsCollections(anEvent, outputEvent);
    }

    void RootPersistencyManager::printEvent(Event* outputEvent) {

        auto particleColl = outputEvent->get<TClonesArray*>("SimParticles", "sim");
        if (!particleColl) {
            throw std::runtime_error("SimParticle output collection is null!");
        }

        if (m_verbose > 1) {
            std::cout << "[ RootPersistencyManager ] : Wrote " << particleColl->GetEntriesFast() << " SimParticle objects" << std::endl;
        }

        if (m_verbose > 2) {
            for (int iColl = 0; iColl < particleColl->GetEntriesFast(); iColl++) {
                particleColl->At(iColl)->Print();
            }
            std::cout << std::endl;
        }

        for (auto entry : outputHitsCollections_) {
            TClonesArray* hitsColl = entry.second;
            int entries = hitsColl->GetEntriesFast();
            if (m_verbose > 1) {
                std::cout << "[ RootPersistencyManager ] : Wrote " << hitsColl->GetEntriesFast() << " hits into " << entry.first << std::endl;
            }
            if (m_verbose > 2) {
                for (int iColl = 0; iColl < entries; iColl++) {
                    TObject* obj = (*hitsColl)[iColl];
                    obj->Print("");
                }
                std::cout << std::endl;
            }
        }
    }

    void RootPersistencyManager::writeHeader(const G4Event* anEvent, Event* outputEvent) {
        EventHeader& eventHeader = ((EventImpl*) outputEvent)->getEventHeaderMutable();

        eventHeader.setEventNumber(anEvent->GetEventID());
        TTimeStamp ts;
        ts.SetSec((int) time(NULL));
        eventHeader.setTimestamp(ts);
        eventHeader.setRun(G4RunManager::GetRunManager()->GetCurrentRun()->GetRunID());
        if (BiasingMessenger::isBiasingEnabled()) {
            eventHeader.setWeight(BiasingMessenger::getEventWeight());
        } else if (anEvent->GetPrimaryVertex(0)) {
            eventHeader.setWeight(anEvent->GetPrimaryVertex(0)->GetWeight());
        }

        std::string seedString = getEventSeeds();
        eventHeader.setStringParameter("eventSeed", seedString);

        if (m_verbose > 1) {
            std::cout << "[ RootPersistencyManager ] : Wrote event header for event ID " << anEvent->GetEventID() << std::endl;
            std::cout << "  ";
            eventHeader.Print("");
        }
    }

    std::string RootPersistencyManager::getEventSeeds(std::string fileName) {
        std::ifstream t(fileName);
        std::stringstream buffer;
        buffer << t.rdbuf();
        t.close();

        return buffer.str();
    }

    void RootPersistencyManager::writeHitsCollections(const G4Event* anEvent, Event* outputEvent) {

        // Clear the hits from last event.
        for (auto entry : outputHitsCollections_) {
            entry.second->Clear("C");
        }

        // Get the HC of this event.
        G4HCofThisEvent* hce = anEvent->GetHCofThisEvent();
        int nColl = hce->GetNumberOfCollections();

        // Loop over all hits collections.
        for (int iColl = 0; iColl < nColl; iColl++) {

            // Get a hits collection and its name.
            G4VHitsCollection* hc = hce->GetHC(iColl);
            std::string collName = hc->GetName();

            if (std::find(dropCollectionNames_.begin(), dropCollectionNames_.end(), collName) 
                    != dropCollectionNames_.end()) {
                if (m_verbose > 1) {  
                    std::cout << "[ RootPersistencyManager ]: Dropping Collection: " << collName << std::endl;
                }
                continue;
            }

            // Get the target output collection.
            TClonesArray* outputHitsColl = outputHitsCollections_[collName];

            // If the collection is not found in the output ROOT event, then a fatal error occurs!
            if (!outputHitsColl) {
                std::cerr << "ERROR: The output collection " << collName << " was not found!" << std::endl;
                G4Exception("RootPersistencyManager::writeHitsCollections", "", FatalException, "The output collection was not found.");
            }

            if (dynamic_cast<G4TrackerHitsCollection*>(hc) != nullptr) {

                // Write G4TrackerHit collection to output SimTrackerHit collection.
                G4TrackerHitsCollection* trackerHitsColl = dynamic_cast<G4TrackerHitsCollection*>(hc);
                writeTrackerHitsCollection(trackerHitsColl, outputHitsColl);

            } else if (dynamic_cast<G4CalorimeterHitsCollection*>(hc) != nullptr) {

                G4CalorimeterHitsCollection* calHitsColl = dynamic_cast<G4CalorimeterHitsCollection*>(hc);
                if (collName == EventConstants::ECAL_SIM_HITS) {
                    // Write ECal G4CalorimeterHit collection to output SimCalorimeterHit collection using helper class.
                    ecalHitIO_->writeHitsCollection(calHitsColl, outputHitsColl);
                } else {
                    // Write generic G4CalorimeterHit collection to output SimCalorimeterHit collection.
                    writeCalorimeterHitsCollection(calHitsColl, outputHitsColl);
                }
            }

            // Add hits collection to output event.
            outputEvent->add(collName, outputHitsColl);
        }
    }

    void RootPersistencyManager::writeTrackerHitsCollection(G4TrackerHitsCollection* hc, TClonesArray* outputColl) {
        int nHits = hc->GetSize();
        for (int iHit = 0; iHit < nHits; iHit++) {
            G4TrackerHit* g4hit = (G4TrackerHit*) hc->GetHit(iHit);
            SimTrackerHit* simTrackerHit = (SimTrackerHit*) outputColl->ConstructedAt(outputColl->GetEntries());
            simTrackerHit->setID(g4hit->getID());
            simTrackerHit->setLayerID(g4hit->getLayerID());
            simTrackerHit->setModuleID(g4hit->getModuleID());
            simTrackerHit->setEdep(g4hit->getEdep());
            const G4ThreeVector& momentum = g4hit->getMomentum();
            simTrackerHit->setMomentum(momentum.x(), momentum.y(), momentum.z());
            const G4ThreeVector& position = g4hit->getPosition();
            simTrackerHit->setEnergy( g4hit->getEnergy() );

            simTrackerHit->setPosition(position.x(), position.y(), position.z());
            simTrackerHit->setPathLength(g4hit->getPathLength());
            simTrackerHit->setTrackID(g4hit->getTrackID());
            simTrackerHit->setPdgID(g4hit->getPdgID());
            SimParticle* simParticle = simParticleBuilder_.findSimParticle(g4hit->getTrackID());
            simTrackerHit->setSimParticle(simParticle);
        }
    }

    void RootPersistencyManager::writeCalorimeterHitsCollection(G4CalorimeterHitsCollection* hc, TClonesArray* outputColl) {
        int nHits = hc->GetSize();
        for (int iHit = 0; iHit < nHits; iHit++) {
            G4CalorimeterHit* g4hit = (G4CalorimeterHit*) hc->GetHit(iHit);
            SimCalorimeterHit* simHit = (SimCalorimeterHit*) outputColl->ConstructedAt(outputColl->GetEntries());
            simHit->setID(g4hit->getID());
            const G4ThreeVector& pos = g4hit->getPosition();
            simHit->setPosition(pos.x(), pos.y(), pos.z());
            SimParticle* particle = simParticleBuilder_.findSimParticle(g4hit->getTrackID());
            simHit->addContrib(particle, g4hit->getPdgCode(), g4hit->getEdep(), g4hit->getTime());
        }
    }

    RunHeader* RootPersistencyManager::createRunHeader(const G4Run* aRun) {

        // Get detector header from the user detector construction.
        DetectorConstruction* detector = ((RunManager*) RunManager::GetRunManager())->getDetectorConstruction();
        DetectorHeader* detectorHeader = detector->getDetectorHeader();

        // Create the run header.
        RunHeader* runHeader 
            = new RunHeader(runNumber_, detectorHeader->getName(), description_);

        // Set parameter value with number of events processed.
        runHeader->setIntParameter("Event count", aRun->GetNumberOfEvent());

        // Set a string parameter with the Geant4 SHA-1.
        G4String g4Version = G4RunManagerKernel::GetRunManagerKernel()->GetVersionString();
        runHeader->setStringParameter("Geant4 revision", g4Version); 

        // Print information about run header.
        if (m_verbose > 1) {
            

            std::ostringstream headerString; 
            headerString << "\n[ RootPersistencyManager ]: Creating run header\n" 
                         << boost::format("\t Run number: %s\n")    % runHeader->getRunNumber() 
                         << boost::format("\t Detector name: %s\n") % runHeader->getDetectorName() 
                         << boost::format("\t Software tag: %s\n")  % runHeader->getSoftwareTag() 
                         << boost::format("\t Description: %s\n")   % runHeader->getDescription()
                         << boost::format("\t Event count: %s\n")   % runHeader->getIntParameter("Event count")
                         << boost::format("\t Geant4 revision: %s\n")  % runHeader->getStringParameter("Geant4 revision"); 
            std::cout << headerString.str() << "\n";  
        }

        return runHeader;
    }

    void RootPersistencyManager::setupHitsCollectionMap() {

        // Clear any state from the last run.
        if (outputHitsCollections_.size()) {
            for (auto entry : outputHitsCollections_) {
                delete entry.second;
            }
        }
        outputHitsCollections_.clear();

        // Create an output TClonesArray in the map for each registered HC.
        G4SDManager* sdMgr = G4SDManager::GetSDMpointer();
        G4HCtable* hcTable = sdMgr->GetHCtable();
        int entries = hcTable->entries();
        for (int i = 0; i < entries; i++) {
            std::string sdName = hcTable->GetSDname(i);
            std::string hcName = hcTable->GetHCname(i);
            G4VSensitiveDetector* sd = sdMgr->FindSensitiveDetector(sdName);
            if (dynamic_cast<CalorimeterSD*>(sd)) {
                outputHitsCollections_[hcName] = new TClonesArray(EventConstants::SIM_CALORIMETER_HIT.c_str(), 500);
                if (m_verbose > 1) {
                    std::cout << "[ RootPersistencyManager ]: Created SimCalorimeterHit HC " << hcName << std::endl;
                }
            } else if (dynamic_cast<TrackerSD*>(sd)) {
                outputHitsCollections_[hcName] = new TClonesArray(EventConstants::SIM_TRACKER_HIT.c_str(), 50);
                if (m_verbose > 1) {
                    std::cout << "[ RootPersistencyManager ]: Created SimTrackerHit HC " << hcName << std::endl;
                }
            } else if (dynamic_cast<ScoringPlaneSD*>(sd)) { 
                outputHitsCollections_[hcName] = new TClonesArray(EventConstants::SIM_TRACKER_HIT.c_str(), 500);
                if (m_verbose > 1) {
                    std::cout << "[ RootPersistencyManager ]: Created ScoringPlaneHit HC " << hcName << std::endl;
                }
            }
        }
    }
} // namespace sim
