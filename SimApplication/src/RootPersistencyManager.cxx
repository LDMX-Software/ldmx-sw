#include "SimApplication/RootPersistencyManager.h"

// LDMX
#include "Event/Event.h"
#include "Event/EventConstants.h"
#include "SimApplication/G4CalorimeterHit.h"
#include "SimApplication/G4TrackerHit.h"

// Geant4
#include "G4RunManager.hh"

// TEmp

//
using event::Event;
using event::EventConstants;

namespace sim {

RootPersistencyManager::RootPersistencyManager()
    : G4PersistencyManager(G4PersistencyCenter::GetPersistencyCenter(), "RootPersistencyManager"), 
            writer_(new RootEventWriter("ldmx_sim_events.root", new SimEvent)) {
    G4PersistencyCenter::GetPersistencyCenter()->RegisterPersistencyManager(this);
    G4PersistencyCenter::GetPersistencyCenter()->SetPersistencyManager(this, "RootPersistencyManager");
    detID = new EcalDetectorID();
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

void RootPersistencyManager::buildEvent(const G4Event* anEvent, Event* outputEvent) {

    // Set basic event information.
    writeHeader(anEvent, outputEvent);
    std::cout << "writing hit collection" << std::endl;
    // Copy hit objects from SD hit collections into the output event.
    writeHitsCollections(anEvent, outputEvent);

    // Set pointer to current G4Event.
    simParticleBuilder_.setCurrentEvent(anEvent);

    // Build the SimParticle list for the output ROOT event.
    std::cout << "building sim particles" << std::endl;
    simParticleBuilder_.buildSimParticles(outputEvent);

    // Assign SimParticle objects to SimTrackerHits.
    std::cout << "assigning tracker hits" << std::endl;
    simParticleBuilder_.assignTrackerHitSimParticles();

    // Assign SimParticle objects to SimCalorimeterHits.
    std::cout << "assigning calor hits" << std::endl;
    simParticleBuilder_.assignCalorimeterHitSimParticles();
}

void RootPersistencyManager::printEvent(Event* outputEvent) {

    // verbose level 2
    if (m_verbose > 1) {
        // Print event number and collection sizes.
        std::cout << std::endl;
        std::cout << "Wrote event " << outputEvent->getEventNumber() << std::endl;
        std::cout << EventConstants::SIM_PARTICLES        << ": " << outputEvent->getCollection(EventConstants::SIM_PARTICLES)->GetEntries() << std::endl;
        std::cout << EventConstants::RECOIL_SIM_HITS      << ": " << outputEvent->getCollection(EventConstants::RECOIL_SIM_HITS)->GetEntries() << std::endl;
        std::cout << EventConstants::TAGGER_SIM_HITS      << ": " << outputEvent->getCollection(EventConstants::TAGGER_SIM_HITS)->GetEntries() << std::endl;
        std::cout << EventConstants::ECAL_SIM_HITS        << ": " << outputEvent->getCollection(EventConstants::ECAL_SIM_HITS)->GetEntries() << std::endl;
        std::cout << EventConstants::HCAL_SIM_HITS        << ": " << outputEvent->getCollection(EventConstants::HCAL_SIM_HITS)->GetEntries() << std::endl;
        std::cout << EventConstants::TRIGGER_PAD_SIM_HITS << ": " << outputEvent->getCollection(EventConstants::TRIGGER_PAD_SIM_HITS)->GetEntries() << std::endl;
        std::cout << EventConstants::TARGET_SIM_HITS      << ": " << outputEvent->getCollection(EventConstants::TARGET_SIM_HITS)->GetEntries() << std::endl;
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
        	//Select ECAL hit collection and perform simulated hit readout
			if (collName == EventConstants::ECAL_SIM_HITS){
				std::pair<std::map<layer_cell_pair, int>::iterator, bool> isInserted;
				for (int iHit = 0; iHit < nHits; iHit++) {
					G4CalorimeterHit* g4hit = (G4CalorimeterHit*) hc->GetHit(iHit);
					int detIDraw= g4hit->getID();
					detID->setRawValue(detIDraw);
					detID->unpack();
					int layer = detID->getFieldValue("layer");
					int cellid = detID->getFieldValue("cellid");

					std::pair<layer_cell_pair,int> layer_cell_index =
							std::make_pair(std::make_pair(layer,cellid),outputColl->GetEntries());
					isInserted = ecalReadoutMap.insert(layer_cell_index);

					SimCalorimeterHit* simHit;
					if (isInserted.second == false){
						simHit = (SimCalorimeterHit*) outputColl->At(isInserted.first->second );
					} else{
						simHit = (SimCalorimeterHit*) outputColl->ConstructedAt(outputColl->GetEntries());
					}
					g4hit->setSimCalorimeterHit(simHit,!isInserted.second); /* copy data from G4 hit to readout hit */
				}
				ecalReadoutMap.clear();
			} else {         	//For now all other cases are considered at once.
								//Future code should read out HCAL_SIM_HITS separately from other Calor derived hits
				for (int iHit = 0; iHit < nHits; iHit++) {
					G4CalorimeterHit* g4hit = (G4CalorimeterHit*) hc->GetHit(iHit);
					SimCalorimeterHit* simHit = (SimCalorimeterHit*) outputColl->ConstructedAt(outputColl->GetEntries());
					g4hit->setSimCalorimeterHit(simHit); /* copy data from G4 hit to sim hit */
				}
			}
        }
    }
}

} // namespace sim
