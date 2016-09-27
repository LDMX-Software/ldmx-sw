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
#include "TH1F.h"
#include "TH2F.h"

// STL
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

int main(int argc, const char* argv[])  {

    if (argc < 2) {
        std::cerr << "ERROR: Missing name of input file." << std::endl;
        exit(1);
    }

    TFile histFile("ldmx_sim_histos.root", "new");
    //TH1F *h1 = new TH1F("h1_nRecoilHits", "Number of Recoil Hits", 100, -0.5, 99.5);
    TH1F* nTaggerHitsHist = new TH1F("h1_nTaggerHits", "Number of Recoil Hits", 100, -0.5, 99.5);
    TH2F* taggerXYHist = new TH2F("h2_taggerXY", "Tagger Hits XY", 200, -100, 100, 200, -100, 100);

    std::cout << "Reading in ROOT file " << argv[1] << " ... " << std::endl;
    TFile* eventFile = new TFile(argv[1]);
    TTree* eventTree = (TTree*) eventFile->Get("LDMX_Event");
    eventTree->Print();
    Event* currentEvent = new Event();
    TBranch *eventBranch = eventTree->GetBranch("LdmxEvent");
    if (eventBranch == NULL) {
        std::cerr << "The LdmxEvent branch is null!" << std::endl;
        exit(1);
    }
    eventBranch->SetAddress(&currentEvent);

    for(int entry = 0; entry < eventTree->GetEntries(); ++entry) {

        std::cout << "Getting TTree entry " << entry << std::endl;

        eventTree->GetEntry(entry);

        std::cout << "EventHeader: run = " << currentEvent->getHeader()->getRun() << "; eventNumber = "
                << currentEvent->getHeader()->getEventNumber() << "; timestamp = " << currentEvent->getHeader()->getTimestamp() << std::endl;

        TClonesArray* coll = NULL;

        //std::cout << "Event has " << event->getCollectionSize("EcalSimHits") << " EcalSimHits" << std::endl;
        //TClonesArray* coll = event->getCollection("EcalSimHits");
        //for (int i = 0; i < event->getCollectionSize("EcalSimHits"); i++) {
        //    coll[i].Print();
        //}

        int nSimParticles = currentEvent->getCollectionSize(Event::SIM_PARTICLES);
        std::cout << "Event has " << nSimParticles << " " << Event::SIM_PARTICLES << std::endl;
        coll = currentEvent->getCollection(Event::SIM_PARTICLES);
        for (int i = 0; i < nSimParticles; i++) {
            std::cout << "Printing SimParticle " << i << std::endl;
            coll->At(i)->Print();
        }

        int nTaggerHits = currentEvent->getCollectionSize(Event::TAGGER_SIM_HITS);
        std::cout << "Event has " << nTaggerHits << " " << Event::TAGGER_SIM_HITS << std::endl;

        nTaggerHitsHist->Fill(nTaggerHits);

        coll = currentEvent->getCollection(Event::TAGGER_SIM_HITS);
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

            taggerXYHist->Fill(simTrackerHit->getStartPosition()[0], simTrackerHit->getStartPosition()[1]);
        }

        int nRecoilHits = currentEvent->getCollectionSize(Event::RECOIL_SIM_HITS);
        std::cout << "Event has " << nRecoilHits << " " << Event::RECOIL_SIM_HITS << std::endl;
        coll = currentEvent->getCollection(Event::RECOIL_SIM_HITS);
        for (int i = 0; i < nRecoilHits; i++) {
            std::cout << "Printing " << Event::RECOIL_SIM_HITS << "[" << i << "]" << std::endl;
            coll->At(i)->Print();
        }

        std::cout << "Done reading event " << currentEvent->getHeader()->getEventNumber() << std::endl << std::endl;
    }

    histFile.Write();
    histFile.Close();

    return 0;
}
