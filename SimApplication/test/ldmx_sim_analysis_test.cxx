#include "Event/Event.h"

// LDMX
#include "Event/Event.h"
#include "Event/EventConstants.h"
#include "Event/SimCalorimeterHit.h"
#include "Event/SimParticle.h"
#include "Event/SimTrackerHit.h"
#include "DetDescr/DetectorID.h"
#include "DetDescr/IDField.h"

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
#include <bitset>

using event::Event;
using detdescr::DetectorID;
using detdescr::IDField;

int main(int argc, const char* argv[])  {

    if (argc < 2) {
        std::cerr << "ERROR: Missing name of input file." << std::endl;
        exit(1);
    }

    // Define DetectorID for decoding the hit IDs.
    IDField::IDFieldList fieldList;
    fieldList.push_back(new IDField("subdet", 0, 0, 3));
    fieldList.push_back(new IDField("layer", 1, 4, 11));
    DetectorID* detID = new DetectorID(&fieldList);

    TFile histFile("ldmx_sim_histos.root", "recreate");

    TH1F* nTaggerHitsH1 = new TH1F("h1_nTaggerHits", "Number of Tagger Hits", 100, 0, 100);
    TH2F* taggerXYH2 = new TH2F("h2_taggerXY", "Tagger Hits XY", 200, -100, 100, 200, -100, 100);

    TH1F* nRecoilHitsH1 = new TH1F("h1_nRecoilHits", "Number of Recoil Hits", 100, 0, 100);
    TH2F* recoilXYH2 = new TH2F("h2_recoilXY", "Recoil Hits XY", 200, -100, 100, 200, -100, 100);
    TH1F* recoilLayer = new TH1F("h1_recoilLayer", "Recoil Layer Number", 8, -0.5, 7.5);

    TH1F* nEcalHitsH1 = new TH1F("h1_nEcalHits", "Number of ECal Hits", 1500, 500, 5000);

    TH1F* nHcalHitsH1 = new TH1F("h1_nHcalHits", "Number of HCal Hits", 100, 0, 100);

    TH1F* nSimParticlesH1 = new TH1F("h1_nSimParticles", "Number of Sim Particles", 100, 0, 100);

    //std::cout << "Reading in ROOT file " << argv[1] << " ... " << std::endl;
    TFile* file = new TFile(argv[1]);
    TTree* tree = (TTree*) file->Get("LDMX_Event");
    //eventTree->Print();
    Event* event = new Event();
    TBranch *branch = tree->GetBranch("LdmxEvent");
    if (branch == NULL) {
        std::cerr << "The LdmxEvent branch is null!" << std::endl;
        exit(1);
    }
    branch->SetAddress(&event);

    for(int entry = 0; entry < tree->GetEntries(); ++entry) {

        tree->GetEntry(entry);

        int eventNumber = event->getEventNumber();
        int nSimParticles = event->getCollection(event::SIM_PARTICLES)->GetEntries();
        int nTaggerHits = event->getCollection(event::TAGGER_SIM_HITS)->GetEntries();
        int nRecoilHits = event->getCollection(event::RECOIL_SIM_HITS)->GetEntries();
        int nEcalHits = event->getCollection(event::ECAL_SIM_HITS)->GetEntries();
        int nHcalHits = event->getCollection(event::HCAL_SIM_HITS)->GetEntries();
        
        std::cout << ">>> Event " << eventNumber << std::endl;
        std::cout << "  "  << event::SIM_PARTICLES << ": " << nSimParticles << std::endl;
        std::cout << "  "  << event::TAGGER_SIM_HITS << ": " << nTaggerHits << std::endl;
        std::cout << "  "  << event::RECOIL_SIM_HITS << ": " << nRecoilHits << std::endl;
        std::cout << "  "  << event::ECAL_SIM_HITS << ": " << nEcalHits << std::endl;
        std::cout << "  "  << event::HCAL_SIM_HITS << ": " << nHcalHits << std::endl;
        std::cout << std::endl;

        /*
        for (int i = 0; i < event->getCollectionSize("EcalSimHits"); i++) {
            // TODO: ecal plots
        }

        coll = event->getCollection(Event::SIM_PARTICLES);
        for (int i = 0; i < nSimParticles; i++) {
            // TODO: SimParticle plots
        }

        nTaggerHitsH1->Fill(nTaggerHits);

        coll = event->getCollection(Event::TAGGER_SIM_HITS);
        for (int i = 0; i < nTaggerHits; i++) {
            SimTrackerHit* simTrackerHit = (SimTrackerHit*) coll->At(i);
            taggerXYH2->Fill(simTrackerHit->getStartPosition()[0], simTrackerHit->getStartPosition()[1]);
        }

        nRecoilHitsH1->Fill(nRecoilHits);
        coll = event->getCollection(Event::RECOIL_SIM_HITS);
        for (int i = 0; i < nRecoilHits; i++) {

            SimTrackerHit* simTrackerHit = (SimTrackerHit*) coll->At(i);

            detID->setRawValue(simTrackerHit->getID());
            int rawID = simTrackerHit->getID();

            detID->unpack();
            int layer = detID->getFieldValue("layer");

            recoilLayer->Fill(layer);

            recoilXYH2->Fill(simTrackerHit->getStartPosition()[0], simTrackerHit->getStartPosition()[1]);
        }
        */
    }

    histFile.Write();
    histFile.Close();

    return 0;
}
