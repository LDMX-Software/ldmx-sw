#include "Event/Event.h"

// LDMX
#include "Event/SimCalorimeterHit.h"
#include "Event/SimParticle.h"
#include "Event/SimTrackerHit.h"

// ROOT
#include "TClonesArray.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TSystem.h"

// STL
#include <stdio.h>
#include <iostream>

int main(int, const char* argv[])  {

    std::cout << "Hello LDMX Event Model Test!" << std::endl;

    //std::cout << "Loading libEvent.so" << std::endl;
    //gSystem->Load("libEvent.so");

    std::cout << "Opening ROOT file for writing" << std::endl;

    /*
     * Open ROOT file for writing.
     */
    TFile* rootFile = new TFile("ldmx_event_model_test.root", "RECREATE");
    TTree *tree = new TTree("LDMX_Event", "LDMX event tree");
    Event* event = new Event();
    tree->Branch("Event", "Event", &event, 32000, 3);

    std::cout << "Creating a dummy event" << std::endl;

    /*
     * Create a dummy event.
     */
    event->setTimestamp(1473280918);
    event->setRun(1);
    event->setEventNumber(1000);
    event->setWeight(1.23);

    std::cout << "Creating SimCalorimeterHit" << std::endl;
    SimCalorimeterHit* calHit = (SimCalorimeterHit*) event->addObject("EcalSimHits");
    calHit->setEdep(1.234);
    calHit->setID(11111111L);
    calHit->setPosition(10., 20., 5000.);

    std::cout << "Creating SimTrackerHit" << std::endl;
    SimTrackerHit* trackerHit = (SimTrackerHit*) event->addObject("RecoilSimHits");
    trackerHit->setEdep(2.345);
    trackerHit->setEndPosition(50., 40., 2000.);
    trackerHit->setStartPosition(60., 50., 2001.);
    trackerHit->setID(22222222L);
    trackerHit->setMomentum(1.0, 2.0, 3.0);
    trackerHit->setTime(42.);

    std::cout << "Creating SimParticle #1" << std::endl;
    SimParticle* particle1 = (SimParticle*) event->addObject("SimParticles");
    particle1->setEndPoint(60., 70., 2002.);
    particle1->setEnergy(3.45);
    particle1->setGenStatus(1);
    particle1->setSimStatus(2);
    particle1->setMass(85.);
    particle1->setMomentum(80., 90., 2003.);
    particle1->setPdg(11);
    particle1->setTime(69.);
    particle1->setVertex(90., 100., 2004.);

    /*
    std::cout << "Creating SimParticle #2" << std::endl;
    SimParticle* particle2 = (SimParticle*) event->addObject("SimParticles");
    particle2->setEndPoint(100., 100., 2016.);
    particle2->setEnergy(4.56);
    particle2->setGenStatus(2);
    particle2->setSimStatus(3);
    particle2->setMass(2.);
    particle2->setMomentum(100., 101., 1001.);
    particle2->setPdg(10);
    particle2->setTime(100.);
    particle2->setVertex(0., 0., 90.);

    std::cout << "Adding daughter to SimParticle #1" << std::endl;
    particle1->addDaughter(particle2);

    std::cout << "Adding parent to SimParticle #2" << std::endl;
    particle2->addParent(particle1);
    */

    std::cout << "Event: run = " << event->getRun() << "; eventNum = " << event->getEventNumber() << "; timestamp = " << event->getTimestamp() << std::endl;
    std::cout << "Created event has " << event->getCollectionSize("EcalSimHits") << " EcalSimHits" << std::endl;
    std::cout << "Created event has " << event->getCollectionSize("RecoilSimHits") << " RecoilSimHits" << std::endl;
    std::cout << "Created event has " << event->getCollectionSize("SimParticles") << " SimParticles" << std::endl;

    /**
     * Fill the ROOT tree with event data.
     */
    std::cout << "Filling ROOT tree" << std::endl;
    tree->Fill();

    /**
     * Write out the ROOT file.
     */
    std::cout << "Writing ROOT file" << std::endl;
    rootFile->Write();
    rootFile->Close();

    //delete rootFile;
    delete tree;
    delete event;

    /**
     * Read the ROOT file back in.
     */
    std::cout << "Reading ROOT file" << std::endl;
    TFile *readFile = new TFile("ldmx_event_model_test.root");

    std::cout << "Getting LDMX_Event TTree" << std::endl;
    TTree *readTree = (TTree*) readFile->Get("LDMX_Event");

    std::cout << "Loading Event from TBranch" << std::endl;
    Event* readEvent = new Event();
    std::cout << "Calling GetBranch" << std::endl;
    TBranch *branch = readTree->GetBranch("Event");
    std::cout << "Calling SetAddress with Event ref" << std::endl;
    branch->SetAddress(&readEvent);

    std::cout << "Reading event entries" << std::endl;
    for(int entry = 0; entry < tree->GetEntries(); ++entry) {
        std::cout << "Reading entry " << entry << " from file" << std::endl;
        tree->GetEntry(entry);

        std::cout << "Read Event: run = " << event->getRun() << "; eventNum = " << event->getEventNumber() << "; timestamp = " << event->getTimestamp() << std::endl;

        std::cout << "Read event has " << event->getCollectionSize("EcalSimHits") << " EcalSimHits" << std::endl;

        TClonesArray* coll = event->getCollection("EcalSimHits");
        for (int i = 0; i < event->getCollectionSize("EcalSimHits"); i++) {
            coll[i].Print();
        }

        std::cout << "Read event has " << event->getCollectionSize("RecoilSimHits") << " RecoilSimHits" << std::endl;
        coll = event->getCollection("RecoilSimHits");
        for (int i = 0; i < event->getCollectionSize("RecoilSimHits"); i++) {
            coll[i].Print();
        }

        std::cout << "Read event has " << event->getCollectionSize("SimParticles") << " SimParticles" << std::endl;
        coll = event->getCollection("SimParticles");
        for (int i = 0; i < event->getCollectionSize("SimParticles"); i++) {
            coll[i].Print();
        }

        readEvent->Clear("C");

        std::cout << std::endl;
    }

    //delete readFile;
    //delete readTree;
    //delete readEvent;

    std::cout << "Bye LDMX Event Model Test!" << std::endl;

    return 0;
}
