#include "SimApplication/RootPersistencyManager.h"

// LDMX
#include "Event/Event.h"
#include "Event/EventConstants.h"
#include "SimApplication/EcalHitIO.h"
#include "SimApplication/G4TrackerHit.h"

// Geant4
#include "G4RunManager.hh"

using event::Event;
using event::EventConstants;
using sim::EcalHitIO;

namespace sim {

RootPersistencyManager::RootPersistencyManager() :
        G4PersistencyManager(G4PersistencyCenter::GetPersistencyCenter(), "RootPersistencyManager"),
        writer_(new RootEventWriter("ldmx_sim_events.root", new SimEvent)),
        ecalHitIO_(new EcalHitIO(&simParticleBuilder_)) {
    G4PersistencyCenter::GetPersistencyCenter()->RegisterPersistencyManager(this);
    G4PersistencyCenter::GetPersistencyCenter()->SetPersistencyManager(this,"RootPersistencyManager");
}

G4bool RootPersistencyManager::Store(const G4Event* anEvent) {

    // verbose level 2
    if (m_verbose > 1) {
        std::cout << "RootPersistencyManager::Store - event " << anEvent->GetEventID() << std::endl;
    }

    // Get the current output event from the writer.
    Event* outputEvent = writer_->getEvent();

    if (G4RunManager::GetRunManager()->GetCurrentEvent()->IsAborted()) {
        outputEvent->Clear("");
        return false;
    }

    // Build the output collections.
    buildEvent(anEvent, outputEvent);

    // Print out event info and data depending on verbose level.
    printEvent(outputEvent);

    // Fill the current ROOT event into the tree branch.
    writer_->writeEvent();

    return true;
}

void RootPersistencyManager::buildEvent(const G4Event* anEvent,Event* outputEvent) {

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

    // verbose level 2
    if (m_verbose > 1) {
        // Print output event collection sizes.
        outputEvent->Print();
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
        for (Event::CollectionMap::const_iterator iColl = collMap.begin();
                iColl != collMap.end(); iColl++) {
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

    // Get the HC of this event.
    G4HCofThisEvent* hce = anEvent->GetHCofThisEvent();
    if (hce == nullptr) {
        throw std::runtime_error("The HCE of this event is null!");
    }
    int nColl = hce->GetNumberOfCollections();

    // Loop over all hits collections.
    for (int iColl = 0; iColl < nColl; iColl++) {

        // Get a hits collection and its name.
        G4VHitsCollection* hc = hce->GetHC(iColl);
        std::string collName = hc->GetName();

        // Get the target output collection.
        TClonesArray* outputColl = outputEvent->getCollection(hc->GetName());

        // If the collection is not found in the output ROOT event, then a fatal error occurs!
        if (!outputColl) {
            std::cerr << "ERROR: The output collection " << collName << " was not found!" << std::endl;
            G4Exception("RootPersistencyManager::writeHitsCollections",
                    "",
                    FatalException,
                    "The output collection " + std::string(collName).c_str() + " was not found.");
        }

        if (dynamic_cast<G4TrackerHitsCollection*>(hc) != nullptr) {

            // Write G4TrackerHit collection to output SimTrackerHit collection.
            G4TrackerHitsCollection* trackerHitsColl = dynamic_cast<G4TrackerHitsCollection*>(hc);
            writeTrackerHitsCollection(trackerHitsColl, outputColl);

        } else if (dynamic_cast<G4CalorimeterHitsCollection*>(hc) != nullptr) {

            G4CalorimeterHitsCollection* calHitsColl = dynamic_cast<G4CalorimeterHitsCollection*>(hc);

            if (collName == EventConstants::ECAL_SIM_HITS) {

                // Write ECal G4CalorimeterHit collection to output SimCalorimeterHit collection using helper class.
                ecalHitIO_->writeHitsCollection(calHitsColl, outputColl);

            } else {

                // Write generic G4CalorimeterHit collection to output SimCalorimeterHit collection.
                writeCalorimeterHitsCollection(calHitsColl, outputColl);
            }
        }
    }
}

void RootPersistencyManager::writeTrackerHitsCollection(G4TrackerHitsCollection* hc, TClonesArray* outputColl) {
    int nHits = hc->GetSize();
    for (int iHit = 0; iHit < nHits; iHit++) {
        G4TrackerHit* g4hit = (G4TrackerHit*) hc->GetHit(iHit);
        SimTrackerHit* simTrackerHit = (SimTrackerHit*) outputColl->ConstructedAt(outputColl->GetEntries());
        simTrackerHit->setID(g4hit->getID());
        simTrackerHit->setLayerID(g4hit->getLayerID());
        simTrackerHit->setEdep(g4hit->getEdep());
        simTrackerHit->setTime(g4hit->getTime());
        const G4ThreeVector& momentum = g4hit->getMomentum();
        simTrackerHit->setMomentum(
                momentum.x(),
                momentum.y(),
                momentum.z());
        const G4ThreeVector& position = g4hit->getPosition();
        simTrackerHit->setPosition(
                position.x(),
                position.y(),
                position.z());
        simTrackerHit->setPathLength(g4hit->getPathLength());
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

} // namespace sim
