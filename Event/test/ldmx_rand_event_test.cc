// LDMX
#include "Event/Event.h"
#include "Event/SimCalorimeterHit.h"
#include "Event/SimParticle.h"
#include "Event/SimTrackerHit.h"

// ROOT
#include "TClonesArray.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"

// STL
#include <ctime>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

float randFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

float randInt(int min, int max) {
    return rand() % (max - min + 1) + min;
}

double randDouble(double min, double max) {
    return min + static_cast<double>(rand()) / (static_cast<double>(RAND_MAX / (max - min)));
}

void fillRand(SimTrackerHit* trackerHit) {
    trackerHit->setEdep(randDouble(0.1, 1.0));
    trackerHit->setEndPosition(randFloat(0., 10), randFloat(0., 20), randFloat(2000., 5000.));
    trackerHit->setStartPosition(randFloat(0., 10), randFloat(0., 20), randFloat(2000., 5000.));
    trackerHit->setMomentum(randFloat(0., 5.0), randFloat(0., 5.0), randFloat(0., 5.0));
    trackerHit->setTime(randFloat(1.0, 100.0));
}

void fillRand(SimCalorimeterHit* calHit) {
    calHit->setEdep(randDouble(0.1, 5.0));
    calHit->setPosition(randFloat(0., 10), randFloat(0., 20), randFloat(2000., 5000.));
}

void fillRand(SimParticle* particle) {
    particle->setEndPoint(randFloat(0., 10), randFloat(0., 20), randFloat(2000., 5000.));
    particle->setEnergy(randDouble(1.0, 10.0));
    particle->setGenStatus(randInt(1, 5));
    particle->setSimStatus(randInt(1, 5));
    particle->setMass(randDouble(0.5, 2.0));
    particle->setMomentum(randFloat(0., 5.0), randFloat(0., 5.0), randFloat(0., 5.0));
    particle->setPdg(randInt(0, 15));
    particle->setTime(randFloat(1., 69.));
    particle->setVertex(randFloat(0., 10), randFloat(0., 20), randFloat(2000., 5000.));
}

int main(int argc, const char* argv[])  {

    std::cout << "Hello LDMX Rand Event Test!" << std::endl;

    srand(time(NULL));

    int nEvents = 1000;
    if (argc > 1) {
        nEvents = atoi(argv[1]);
    }

    /*
     * Open ROOT file for writing.
     */
    TFile* rootFile = new TFile("ldmx_rand_event_test.root", "RECREATE");
    TTree *tree = new TTree("LDMX_Event", "LDMX event tree");
    Event* event = new Event();
    tree->Branch("Event", "Event", &event, 32000, 3);

    std::cout << "Test will generate " << nEvents << " random events ..." << std::endl;

    for (int iEvent = 0; iEvent < nEvents; iEvent++) {

        std::cout << "Generating rand event " << iEvent << " ..." << std::endl;

        int nCalHits = randInt(10, 100);
        int nTrackerHits = randInt(10, 100);
        int nParticles = randInt(1, 10);

        EventHeader* header = event->getHeader();
        header->setRun(1);
        header->setTimestamp(std::time(0));
        header->setEventNumber(iEvent);

        std::cout << "Making " << nCalHits << " rand SimCalorimeterHits" << std::endl;
        for (int iCalHit = 0; iCalHit < nCalHits; iCalHit++) {
            SimCalorimeterHit* calHit = (SimCalorimeterHit*) event->addObject("EcalSimHits");
            calHit->setID((long) (iCalHit + 1));
            fillRand(calHit);
        }

        std::cout << "Making " << nTrackerHits << " rand SimCalorimeterHits" << std::endl;
        for (int iTrackerHit = 0; iTrackerHit < nTrackerHits; iTrackerHit++) {
            SimTrackerHit* trackerHit = (SimTrackerHit*) event->addObject("RecoilSimHits");
            trackerHit->setID((long) (iTrackerHit + 1));
            fillRand(trackerHit);
        }

        std::cout << "Making " << nTrackerHits << " rand SimParticles" << std::endl;
        for (int iParticle = 0; iParticle < nParticles; iParticle++) {
            SimParticle* simParticle = (SimParticle*) event->addObject("SimParticles");
            fillRand(simParticle);
        }

        /*
         * Write the event to the tree.
         */
        tree->Fill();

        /*
         * Clear the event.
         */
        event->Clear();
    }

    std::cout << "Writing ROOT file ..." << std::endl;
    rootFile->Write();
    rootFile->Close();

    std::cout << "Bye LDMX Rand Event Test!" << std::endl;
}

