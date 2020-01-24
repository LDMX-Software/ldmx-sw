/**
 * @file RootPersistencyManager.cxx
 * @brief Class used to manage ROOT based persistency.
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimApplication/RootPersistencyManager.h"

namespace ldmx {

    RootPersistencyManager::RootPersistencyManager() :
        G4PersistencyManager(G4PersistencyCenter::GetPersistencyCenter(), "RootPersistencyManager")
    {
        G4PersistencyCenter::GetPersistencyCenter()->RegisterPersistencyManager(this);
        G4PersistencyCenter::GetPersistencyCenter()->SetPersistencyManager(this, "RootPersistencyManager");

        event_ = new Event("sim");
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
        outputFile_->setupEvent(event_);
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

        std::map<int,SimParticle> particleColl = outputEvent->getMap<int,SimParticle>( "SimParticles", "sim");

        if ( m_verbose > 1 ) {
            std::cout << "[ RootPersistencyManager ] : Event Bus Contents" << std::endl;
            outputEvent->Print( m_verbose );
            std::cout << "[ RootPersistencyManager ] : End Event Bus Contents" << std::endl;
        }

    }

    void RootPersistencyManager::writeHeader(const G4Event* anEvent, Event* outputEvent) {

        EventHeader& eventHeader = outputEvent->getEventHeader();

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

        outputEvent->add( EventConstants::EVENT_HEADER , eventHeader );
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
