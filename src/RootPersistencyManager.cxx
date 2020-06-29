
#include "SimCore/RootPersistencyManager.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <algorithm>
#include <memory> 

/*~~~~~~~~~~~*/
/*   Event   */
/*~~~~~~~~~~~*/
#include "Event/EventConstants.h"
#include "Event/EventHeader.h"
#include "Event/RunHeader.h"
#include "Event/SimTrackerHit.h" 

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/DetectorConstruction.h"
#include "SimCore/RunManager.h"
#include "SimCore/UserEventInformation.h" 

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4RunManagerKernel.hh"

namespace ldmx {

    RootPersistencyManager::RootPersistencyManager(EventFile &file, Parameters& parameters) :
        G4PersistencyManager(G4PersistencyCenter::GetPersistencyCenter(), "RootPersistencyManager"),
        file_(file) {

        // Let Geant4 know what to use this persistency manager
        G4PersistencyCenter::GetPersistencyCenter()->RegisterPersistencyManager(this);
        G4PersistencyCenter::GetPersistencyCenter()->SetPersistencyManager(this, "RootPersistencyManager");

        // Set the description 
        description_ = parameters.getParameter< std::string >("description"); 
        runNumber_   = parameters.getParameter< int >("runNumber");
        
        ecalHitIO_.setEnableHitContribs(parameters.getParameter< bool >("enableHitContribs")); 
        ecalHitIO_.setCompressHitContribs(parameters.getParameter< bool >("compressHitContribs"));

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
        RunHeader runHeader( runNumber_ , detector->getDetectorHeader()->getName(), description_ );

        // Set parameter value with number of events processed.
        runHeader.setIntParameter("Event Count", eventsCompleted_ );
        runHeader.setIntParameter("Events Began" , eventsBegan_ );
        
        // Set a string parameter with the Geant4 SHA-1.
        G4String g4Version{G4RunManagerKernel::GetRunManagerKernel()->GetVersionString()};
        runHeader.setStringParameter("Geant4 revision", g4Version); 

        // Write the header to the file.
        file_.writeRunHeader(runHeader);

        return true;
    }

    void RootPersistencyManager::Initialize() {
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

    void RootPersistencyManager::writeHeader(const G4Event* anEvent) {

        // Retrieve a mutable version of the event header
        EventHeader& eventHeader = event_->getEventHeader();

        // Set the event weight
        double weight{1}; 
        if (anEvent->GetUserInformation() != nullptr) { 
            weight = static_cast<UserEventInformation*>(anEvent->GetUserInformation())->getWeight(); 
        } else if (anEvent->GetPrimaryVertex(0)) {
            weight = anEvent->GetPrimaryVertex(0)->GetWeight(); 
        }
        eventHeader.setWeight(weight); 

        // Save the state of the random engine to an output stream. A string
        // is then extracted and saved to the event header.   
        std::ostringstream stream;
        G4Random::saveFullState(stream);
        //std::cout << stream.str() << std::endl;
        eventHeader.setStringParameter("eventSeed", stream.str());
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

} // namespace ldmx
