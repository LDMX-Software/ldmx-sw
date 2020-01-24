/**
 * @file RootPersistencyManager.cxx
 * @brief Class used to manage ROOT based persistency.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimApplication/RootPersistencyManager.h"

//----------------//
//   C++ StdLib   //
//----------------//
#include <algorithm>
#include <memory> 

//-----------//
//   Boost   //
//-----------//
#include "boost/format.hpp"

//-------------//
//   ldmx-sw   //
//-------------//
#include "Exception/Exception.h"
#include "Event/Event.h"
#include "Event/EventHeader.h"
#include "Event/RunHeader.h"
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

    RootPersistencyManager::RootPersistencyManager(EventFile &file) :
        G4PersistencyManager(G4PersistencyCenter::GetPersistencyCenter(), "RootPersistencyManager"),
        file_(file),  
        ecalHitIO_(new EcalHitIO(&simParticleBuilder_)) 
    {
        G4PersistencyCenter::GetPersistencyCenter()->RegisterPersistencyManager(this);
        G4PersistencyCenter::GetPersistencyCenter()->SetPersistencyManager(this, "RootPersistencyManager");

    }

    G4bool RootPersistencyManager::Store(const G4Event* anEvent) {

        // Check if the event has been aborted.  If so, skip storage of the 
        // event. 
        if (G4RunManager::GetRunManager()->GetCurrentEvent()->IsAborted()) return false; 

        // Build the output collections.
        buildEvent(anEvent);

        return true;
    }

    G4bool RootPersistencyManager::Store(const G4Run* aRun) {
      
        // NOTE: This method is called once the run is terminated through 
        // the run manager.  

        // Get the detector header from the user detector construction
        auto detector 
            = static_cast<RunManager*>(RunManager::GetRunManager())->getDetectorConstruction();

        // Create the run header.
        auto runHeader  
            = std::make_unique<RunHeader>(runNumber_, detector->getDetectorHeader()->getName(), description_);

        // Set parameter value with number of events processed.
        runHeader->setIntParameter("Event count", aRun->GetNumberOfEvent());

        // Set a string parameter with the Geant4 SHA-1.
        G4String g4Version{G4RunManagerKernel::GetRunManagerKernel()->GetVersionString()};
        runHeader->setStringParameter("Geant4 revision", g4Version); 

        // Write the header to the file.
        file_.writeRunHeader(runHeader.get());  

        return true;
    }

    void RootPersistencyManager::Initialize() {

        // Create map with output hits collections.
        setupHitsCollectionMap();
    }

    void RootPersistencyManager::buildEvent(const G4Event* anEvent) {

        // Set basic event information.
        writeHeader(anEvent);

        // Set pointer to current G4Event.
        simParticleBuilder_.setCurrentEvent(anEvent);
        
        // Build the SimParticle list for the output ROOT event.
        simParticleBuilder_.buildSimParticles(event_);

        // Copy hit objects from SD hit collections into the output event.
        writeHitsCollections(anEvent, event_);

    }

    void RootPersistencyManager::printEvent(Event* outputEvent) {

        std::map<int,SimParticle> particleColl = outputEvent->getMap<int,SimParticle>( "SimParticles", "sim");

        if ( m_verbose > 1 ) {
            std::cout << "[ RootPersistencyManager ] : Event Bus Contents" << std::endl;
            outputEvent->Print( m_verbose );
            std::cout << "[ RootPersistencyManager ] : End Event Bus Contents" << std::endl;
        }

    }

    void RootPersistencyManager::writeHeader(const G4Event* anEvent) {

        // Retrieve a mutable version of the event header
        EventHeader& eventHeader = static_cast<EventImpl*>(event_)->getEventHeaderMutable();

        // Set the event weight
        if (BiasingMessenger::isBiasingEnabled()) { 
            eventHeader.setWeight(BiasingMessenger::getEventWeight());
        } else if (anEvent->GetPrimaryVertex(0)) { 
            eventHeader.setWeight(anEvent->GetPrimaryVertex(0)->GetWeight());
            
        }

        // Set the seeds used for this event
        std::string seedString = getEventSeeds();
        eventHeader.setStringParameter("eventSeed", seedString);
    }

    std::string RootPersistencyManager::getEventSeeds(std::string fileName) {
        std::ifstream t(fileName);
        std::stringstream buffer;
        buffer << t.rdbuf();
        t.close();

        return buffer.str();
    }

    void RootPersistencyManager::writeHitsCollections(const G4Event* anEvent, Event* outputEvent) {

        // Get the HC of this event.
        G4HCofThisEvent* hce = anEvent->GetHCofThisEvent();
        int nColl = hce->GetNumberOfCollections();
        
        // Loop over all hits collections.
        for (int iColl = 0; iColl < nColl; iColl++) {
            
            // Get a hits collection and its name.
            G4VHitsCollection* hc = hce->GetHC(iColl);
            if ( ! hc ) {
                EXCEPTION_RAISE(
                        "G4HitColl",
                        "G4VHitsCollection indexed " + std::to_string(iColl) + " returned a nullptr."
                        );
            }

            std::string collName = hc->GetName();
            
            if (std::find(dropCollectionNames_.begin(), dropCollectionNames_.end(), collName) != dropCollectionNames_.end()) {
                if (m_verbose > 1) {  
                    std::cout << "[ RootPersistencyManager ]: Dropping Collection: " << collName << std::endl;
                }
                continue;
            }

            if (dynamic_cast<G4TrackerHitsCollection*>(hc) != nullptr) {

                // Write G4TrackerHit collection to output SimTrackerHit collection.
                G4TrackerHitsCollection* trackerHitsColl = dynamic_cast<G4TrackerHitsCollection*>(hc);
                std::vector<SimTrackerHit> outputColl;
                writeTrackerHitsCollection( trackerHitsColl, outputColl );
                // Add hits collection to output event.
                outputEvent->add( collName, outputColl );

            } else if (dynamic_cast<G4CalorimeterHitsCollection*>(hc) != nullptr) {

                G4CalorimeterHitsCollection* calHitsColl = dynamic_cast<G4CalorimeterHitsCollection*>(hc);
                std::vector<SimCalorimeterHit> outputColl;
                if (collName == EventConstants::ECAL_SIM_HITS) {
                    // Write ECal G4CalorimeterHit collection to output SimCalorimeterHit collection using helper class.
                    ecalHitIO_.writeHitsCollection(calHitsColl, outputColl );
                } else {
                    // Write generic G4CalorimeterHit collection to output SimCalorimeterHit collection.
                    writeCalorimeterHitsCollection(calHitsColl, outputColl );
                }
                // Add hits collection to output event.
                outputEvent->add( collName, outputColl );
            } //switch on type of hit collection
        } //loop through geant4 hit collections

        return;
    }

    void RootPersistencyManager::writeTrackerHitsCollection(G4TrackerHitsCollection* hc, std::vector<SimTrackerHit> &outputColl) {

        outputColl.clear();
        int nHits = hc->GetSize();
        for (int iHit = 0; iHit < nHits; iHit++) {
            G4TrackerHit* g4hit = (G4TrackerHit*) hc->GetHit(iHit);
            const G4ThreeVector& momentum = g4hit->getMomentum();
            const G4ThreeVector& position = g4hit->getPosition();

            SimTrackerHit simTrackerHit;
            simTrackerHit.setID(         g4hit->getID()         );
            simTrackerHit.setTime(       g4hit->getTime()       );
            simTrackerHit.setLayerID(    g4hit->getLayerID()    );
            simTrackerHit.setModuleID(   g4hit->getModuleID()   );
            simTrackerHit.setEdep(       g4hit->getEdep()       );
            simTrackerHit.setEnergy(     g4hit->getEnergy()     );
            simTrackerHit.setPathLength( g4hit->getPathLength() );
            simTrackerHit.setTrackID(    g4hit->getTrackID()    );
            simTrackerHit.setPdgID(      g4hit->getPdgID()      );
            simTrackerHit.setPosition(position.x(), position.y(), position.z());
            simTrackerHit.setMomentum(momentum.x(), momentum.y(), momentum.z());

            outputColl.push_back( simTrackerHit );
        }

        return;
    }

    void RootPersistencyManager::writeCalorimeterHitsCollection(G4CalorimeterHitsCollection* hc, 
            std::vector<SimCalorimeterHit> &outputColl) {
        int nHits = hc->GetSize();
        for (int iHit = 0; iHit < nHits; iHit++) {
            G4CalorimeterHit* g4hit = (G4CalorimeterHit*) hc->GetHit(iHit);
            const G4ThreeVector& pos = g4hit->getPosition();

            SimCalorimeterHit simHit;
            simHit.setID( g4hit->getID() );
            simHit.addContrib( g4hit->getTrackID(), g4hit->getPdgCode(), g4hit->getEdep(), g4hit->getTime() );
            simHit.setPosition( pos.x(), pos.y(), pos.z() );

            outputColl.push_back( simHit );
        }

        return;
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
            auto sd{sdMgr->FindSensitiveDetector(sdName)};
            if (dynamic_cast<CalorimeterSD*>(sd)) {
                outputHitsCollections_[hcName] = new TClonesArray(EventConstants::SIM_CALORIMETER_HIT.c_str(), 500);
            } else if (dynamic_cast<TrackerSD*>(sd)) {
                outputHitsCollections_[hcName] = new TClonesArray(EventConstants::SIM_TRACKER_HIT.c_str(), 50);
            } else if (dynamic_cast<ScoringPlaneSD*>(sd)) { 
                outputHitsCollections_[hcName] = new TClonesArray(EventConstants::SIM_TRACKER_HIT.c_str(), 500);
            }
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

} // namespace ldmx
