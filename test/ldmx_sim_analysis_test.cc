#include "Event/Event.h"

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
#include "TSystem.h"

// STL
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

int main(int argc, const char* argv[])  {

    if (argc < 2) {
        std::cerr << "ERROR: Missing name of input file." << std::endl;
        exit(1);
    }

    std::cout << "Reading in ROOT file " << argv[1] << " ... " << std::endl;
    TFile *file = new TFile(argv[1]);

    TTree *tree = (TTree*) file->Get("LDMX_Event");
    tree->Print();
    Event* event = new Event();
    TBranch *branch = tree->GetBranch("LdmxEvent");
    if (branch == NULL) {
        std::cerr << "The LdmxEvent branch is null!" << std::endl;
        exit(1);
    }
    branch->SetAddress(&event);

    for(int entry = 0; entry < tree->GetEntries(); ++entry) {

        std::cout << "Getting TTree entry " << entry << std::endl;

        tree->GetEntry(entry);

        std::cout << "EventHeader: run = " << event->getHeader()->getRun() << "; eventNumber = "
                << event->getHeader()->getEventNumber() << "; timestamp = " << event->getHeader()->getTimestamp() << std::endl;

        TClonesArray* coll = NULL;

        //std::cout << "Event has " << event->getCollectionSize("EcalSimHits") << " EcalSimHits" << std::endl;
        //TClonesArray* coll = event->getCollection("EcalSimHits");
        //for (int i = 0; i < event->getCollectionSize("EcalSimHits"); i++) {
        //    coll[i].Print();
        //}

        int nSimParticles = event->getCollectionSize(Event::SIM_PARTICLES);
        std::cout << "Event has " << nSimParticles << " " << Event::SIM_PARTICLES << std::endl;
        coll = event->getCollection(Event::SIM_PARTICLES);
        for (int i = 0; i < nSimParticles; i++) {
            std::cout << "Printing SimParticle " << i << std::endl;
            coll->At(i)->Print();
        }

        int nTaggerHits = event->getCollectionSize(Event::TAGGER_SIM_HITS);
        std::cout << "Event has " << nTaggerHits << " " << Event::TAGGER_SIM_HITS << std::endl;
        coll = event->getCollection(Event::TAGGER_SIM_HITS);
        for (int i = 0; i < nTaggerHits; i++) {
            std::cout << "Printing " << Event::TAGGER_SIM_HITS << "[" << i << "]" << std::endl;
            coll->At(i)->Print();

            SimTrackerHit* simTrackerHit = (SimTrackerHit*) coll->At(i);

            std::cout << "Getting SimParticle from SimTrackerHit ..." << std::endl;
            SimParticle* simParticle = simTrackerHit->getSimParticle();
            if (simParticle != NULL) {
                simParticle->Print();
            } else {
                std::cerr << "SimParticle is null!" << std::endl;
            }
        }

        int nRecoilHits = event->getCollectionSize(Event::RECOIL_SIM_HITS);
        std::cout << "Event has " << nRecoilHits << " " << Event::RECOIL_SIM_HITS << std::endl;
        coll = event->getCollection(Event::RECOIL_SIM_HITS);
        for (int i = 0; i < nRecoilHits; i++) {
            std::cout << "Printing " << Event::RECOIL_SIM_HITS << "[" << i << "]" << std::endl;
            coll->At(i)->Print();
        }

        std::cout << "Done reading event " << event->getHeader()->getEventNumber() << std::endl << std::endl;
    }

    return 0;
}
