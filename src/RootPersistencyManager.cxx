#include "SimApplication/RootPersistencyManager.h"

// LDMX
#include "Event/Event.h"
#include "Event/EventConstants.h"
#include "SimApplication/G4CalorimeterHit.h"
#include "SimApplication/G4TrackerHit.h"

// Geant4
#include "G4RunManager.hh"

using event::Event;

namespace sim {

RootPersistencyManager::RootPersistencyManager()
    : G4PersistencyManager(G4PersistencyCenter::GetPersistencyCenter(), "RootPersistencyManager"), 
            writer_(new RootEventWriter("ldmx_sim_events.root", new SimEvent)) {
    G4PersistencyCenter::GetPersistencyCenter()->RegisterPersistencyManager(this);
    G4PersistencyCenter::GetPersistencyCenter()->SetPersistencyManager(this, "RootPersistencyManager");
}

G4bool RootPersistencyManager::Store(const G4Event* anEvent) {

    // verbose level 2
    if (m_verbose > 1) {
        std::cout << "RootPersistencyManager::Store - event " << anEvent->GetEventID() << std::endl;
    }

    // Get the current output event from the writer.
    Event* outputEvent = writer_->getEvent();
    
    // Build the output collections.
    buildEvent(anEvent, outputEvent);
            
    // Fill the current ROOT event into the tree branch.
    writer_->writeEvent();
    
    // Print out event info and data depending on verbose level.
    printEvent(outputEvent);
    
    return true;
}

void RootPersistencyManager::buildEvent(const G4Event* anEvent, Event* outputEvent) {

    // Set basic event information.
    writeHeader(anEvent, outputEvent);

    // Copy hit objects from SD hit collections into the output event.
    writeHitsCollections(anEvent, outputEvent);

    // Set pointer to current G4Event.
    simParticleBuilder_.setCurrentEvent(anEvent);

    // Build the SimParticle list for the output ROOT event.
    simParticleBuilder_.buildSimParticles(outputEvent);

    // Assign SimParticle objects to SimTrackerHits.
    simParticleBuilder_.assignTrackerHitSimParticles();

    // Assign SimParticle objects to SimCalorimeterHits.
    simParticleBuilder_.assignCalorimeterHitSimParticles();
}

void RootPersistencyManager::printEvent(Event* outputEvent) {

    // verbose level 2
    if (m_verbose > 1) {
        // Print event number and collection sizes.
        std::cout << std::endl;
        std::cout << "Wrote event " << outputEvent->getEventNumber() << std::endl;
        std::cout << event::SIM_PARTICLES        << ": " << outputEvent->getCollection(event::SIM_PARTICLES)->GetEntries() << std::endl;
        std::cout << event::RECOIL_SIM_HITS      << ": " << outputEvent->getCollection(event::RECOIL_SIM_HITS)->GetEntries() << std::endl;
        std::cout << event::TAGGER_SIM_HITS      << ": " << outputEvent->getCollection(event::TAGGER_SIM_HITS)->GetEntries() << std::endl;
        std::cout << event::ECAL_SIM_HITS        << ": " << outputEvent->getCollection(event::ECAL_SIM_HITS)->GetEntries() << std::endl;
        std::cout << event::HCAL_SIM_HITS        << ": " << outputEvent->getCollection(event::HCAL_SIM_HITS)->GetEntries() << std::endl;
        std::cout << event::TRIGGER_PAD_SIM_HITS << ": " << outputEvent->getCollection(event::TRIGGER_PAD_SIM_HITS)->GetEntries() << std::endl;
        std::cout << event::TARGET_SIM_HITS      << ": " << outputEvent->getCollection(event::TARGET_SIM_HITS)->GetEntries() << std::endl;
        std::cout << std::endl;    
    }
    
    // verbose level 3
    if (m_verbose > 2) {
        // Print out the detailed tree info from ROOT with branch sizes.
        writer_->getTree()->Print();
    }
    
    // verbose level 4
    if (m_verbose > 3) {
        // Print out all collection objects via their TObject::Print() method.
        const Event::CollectionMap& collMap = outputEvent->getCollectionMap();
        for (Event::CollectionMap::const_iterator iColl = collMap.begin(); iColl != collMap.end(); iColl++) {
            TClonesArray* coll = (*iColl).second;
            std::cout << std::endl;
            std::cout << (*iColl).first << ": " << coll->GetEntries() << std::endl;
            int nEntries = coll->GetEntries();
            for (int iEntry = 0; iEntry < nEntries; iEntry++) {
                coll->At(iEntry)->Print();
            }
        }
        std::cout << std::endl; 
    }
}

void RootPersistencyManager::writeHeader(const G4Event* anEvent, Event* outputEvent) {
    outputEvent->setEventNumber(anEvent->GetEventID());
    outputEvent->setTimestamp((int) time(NULL));
    outputEvent->setRun(G4RunManager::GetRunManager()->GetCurrentRun()->GetRunID());
    if (anEvent->GetPrimaryVertex(0)) {
        outputEvent->setWeight(anEvent->GetPrimaryVertex(0)->GetWeight());
    }
}

void RootPersistencyManager::writeHitsCollections(const G4Event* anEvent, Event* outputEvent) {
    G4HCofThisEvent* hce = anEvent->GetHCofThisEvent();
    if (hce == nullptr) {
        throw std::runtime_error("The HCE of this event is null!");
    }
    int nColl = hce->GetNumberOfCollections();
    for (int iColl = 0; iColl < nColl; iColl++) {
        G4VHitsCollection* hc = hce->GetHC(iColl);
        std::string collName = hc->GetName();
        int nHits = hc->GetSize();
        TClonesArray* outputColl = outputEvent->getCollection(hc->GetName());
        if (dynamic_cast<G4TrackerHitsCollection*>(hc) != nullptr) {
            for (int iHit = 0; iHit < nHits; iHit++) {
                G4TrackerHit* g4hit = (G4TrackerHit*) hc->GetHit(iHit);
                SimTrackerHit* simHit = (SimTrackerHit*) outputColl->ConstructedAt(outputColl->GetEntries());
                g4hit->setSimTrackerHit(simHit); /* copy data from G4 hit to sim hit */
            }
        } else if (dynamic_cast<G4CalorimeterHitsCollection*>(hc) != nullptr) {
            for (int iHit = 0; iHit < nHits; iHit++) {
                G4CalorimeterHit* g4hit = (G4CalorimeterHit*) hc->GetHit(iHit);
                SimCalorimeterHit* simHit = (SimCalorimeterHit*) outputColl->ConstructedAt(outputColl->GetEntries());
                g4hit->setSimCalorimeterHit(simHit); /* copy data from G4 hit to sim hit */
            }
        }
    }
}

} // namespace sim
