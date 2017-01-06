#include "Event/Event.h"

// LDMX
#include "Event/SimEvent.h"
#include "Event/SimCalorimeterHit.h"
#include "Event/SimParticle.h"
#include "Event/SimTrackerHit.h"
#include "DetDescr/EcalDetectorID.h"
#include "DetDescr/EcalHexReadout.h"
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
#include <iostream>
#include <vector>

using event::SimEvent;
using event::Event;
using event::EventConstants;
using event::SimCalorimeterHit;
using event::SimTrackerHit;
using detdescr::DetectorID;
using detdescr::DefaultDetectorID;
using detdescr::EcalDetectorID;
using detdescr::EcalHexReadout;
using detdescr::IDField;
using std::vector;

int main(int argc, const char* argv[])  {

    if (argc < 2) {
        std::cerr << "ERROR: Missing name of input file." << std::endl;
        exit(1);
    }

    // Define detector IDs for decoding the hit IDs.
    DefaultDetectorID detID;
    EcalDetectorID ecalID;

    // Readout for translating IDs to cells.
    EcalHexReadout ecalReadout;

    // Outut file with histograms.
    TFile histFile("ldmx_sim_histos.root", "recreate");

    /*

    TH1F* nTaggerHitsH1 = new TH1F("h1_nTaggerHits", "Number of Tagger Hits", 100, 0, 100);
    TH2F* taggerXYH2 = new TH2F("h2_taggerXY", "Tagger Hits XY", 200, -100, 100, 200, -100, 100);

    TH1F* nHcalHitsH1 = new TH1F("h1_nHcalHits", "Number of HCal Hits", 100, 0, 100);

    TH1F* nSimParticlesH1 = new TH1F("h1_nSimParticles", "Number of Sim Particles", 100, 0, 100);
    */

    // recoil tracker plots
    TH1F* nRecoilHitsH1 = new TH1F("h1_nRecoilHits", "Number of Recoil Hits", 100, 0, 100);
    TH2F* recoilXYH2 = new TH2F("h2_recoilXY", "Recoil Hits XY", 200, -100, 100, 200, -100, 100);
    TH1F* recoilLayer = new TH1F("h1_recoilLayer", "Recoil Layer Number", 8, -0.5, 7.5);

    // ecal plots
    TH1F* ecalHitEdepH1 = new TH1F("h1_ecalHitEdep", "ECal Edep", 400, 0, 2.);
    //TH1F* ecalHitCellH1 = new TH1F("h1_ecalHitCell", "ECal Hit Cell ID", 12000, 0, 12000);
    //TH1F* ecalContribEdepH1 = new TH1F("h1_ecalContribEdep", "ECal Hit Contrib Edep", 400, 0, 4);
    TH2F* ecalXYH2 = new TH2F("h2_ecalXY", "ECal XY", 200, -100, 100, 200, -100, 100);
    TH2F* ecalZYH2 = new TH2F("h2_ecalZY", "ECal ZY", 300, 200, 500, 200, -100, 100);
    TH1F* ecalTimeH1 = new TH1F("h1_ecalTime", "ECal Time", 200, 0, 10);
    TH1F* ecalLayerH1 = new TH1F("h1_ecalLayer", "ECal Layer", 50, 0, 50);
    TH1F* ecalHitsH1 = new TH1F("h1_ecalHits", "ECal Hits", 30, 30, 60);
    TH1F* ecalContribsH1 = new TH1F("h1_ecalContribs", "ECal Contribs", 100, 0, 100);
    TH1F* ecalContribEdepH1 = new TH1F("h1_ecalContribEdep", "ECal Contrib Edep", 1000, 0, 1);

    // branch setup for getting events
    TFile* file = new TFile(argv[1]);
    TTree* tree = (TTree*) file->Get("LDMX_Event");
    SimEvent* event = new SimEvent();
    TBranch *branch = tree->GetBranch("LdmxEvent");
    if (branch == NULL) {
        std::cerr << "The LdmxEvent branch is null!" << std::endl;
        exit(1);
    }
    branch->SetAddress(&event);

    // event loop
    for(int entry = 0; entry < tree->GetEntries(); ++entry) {

        // get next event
        tree->GetEntry(entry);

        // pointer to current collection
        TClonesArray* coll = nullptr;

        int eventNumber = event->getEventNumber();
        unsigned nSimParticles = event->getCollection(EventConstants::SIM_PARTICLES)->GetEntries();
        unsigned nTaggerHits = event->getCollection(EventConstants::TAGGER_SIM_HITS)->GetEntries();
        unsigned nRecoilHits = event->getCollection(EventConstants::RECOIL_SIM_HITS)->GetEntries();
        unsigned nEcalHits = event->getCollection(EventConstants::ECAL_SIM_HITS)->GetEntries();
        unsigned nHcalHits = event->getCollection(EventConstants::HCAL_SIM_HITS)->GetEntries();
        
        /*
        std::cout << ">>> Event " << eventNumber << std::endl;
        std::cout << "  "  << EventConstants::SIM_PARTICLES << ": " << nSimParticles << std::endl;
        std::cout << "  "  << EventConstants::TAGGER_SIM_HITS << ": " << nTaggerHits << std::endl;
        std::cout << "  "  << EventConstants::RECOIL_SIM_HITS << ": " << nRecoilHits << std::endl;
        std::cout << "  "  << EventConstants::ECAL_SIM_HITS << ": " << nEcalHits << std::endl;
        std::cout << "  "  << EventConstants::HCAL_SIM_HITS << ": " << nHcalHits << std::endl;
        std::cout << std::endl;
        */

        // recoil sim hits
        nRecoilHitsH1->Fill(nRecoilHits);
        coll = event->getCollection(EventConstants::RECOIL_SIM_HITS);
        for (int i = 0; i < nRecoilHits; i++) {

            SimTrackerHit* simTrackerHit = (SimTrackerHit*) coll->At(i);

            detID.setRawValue(simTrackerHit->getID());
            detID.unpack();

            int layer = detID.getFieldValue("layer");

            recoilLayer->Fill(layer);
            recoilXYH2->Fill(simTrackerHit->getPosition()[0], simTrackerHit->getPosition()[1]);
        }

        // ecal sim hits
        coll = event->getCollection(EventConstants::ECAL_SIM_HITS);
        ecalHitsH1->Fill(nEcalHits);
        for (unsigned i = 0; i < nEcalHits; i++) {

            SimCalorimeterHit* calHit = (SimCalorimeterHit*) coll->At(i);
            vector<float> pos = calHit->getPosition();
            float time = calHit->getTime();

            int id = calHit->getID();
            ecalID.setRawValue(id);
            const DetectorID::FieldValueList& fieldValues = ecalID.unpack();
            int layer = fieldValues[1];
            int cell = fieldValues[2];

            ecalHitEdepH1->Fill(calHit->getEdep());
            ecalXYH2->Fill(pos[0], pos[1]);
            ecalZYH2->Fill(pos[2], pos[1]);
            ecalTimeH1->Fill(time);
            ecalLayerH1->Fill(layer);

            unsigned nContribs = calHit->getNumberOfContribs();
            ecalContribsH1->Fill(nContribs);

            for (int iContrib = 0; iContrib < nContribs; iContrib++) {
                SimCalorimeterHit::Contrib contrib = calHit->getContrib(iContrib);
                ecalContribEdepH1->Fill(contrib.edep);
                if (!contrib.particle) {
                    std::cout << "WARNING: SimParticle from contrib is null!!!" << std::endl;
                }
            }
        }

            //if (cell < minEcalCell) {
            //    minEcalCell = cell;
            //} else if (cell > maxEcalCell) {
            //    maxEcalCell = cell;
            //}

            /*
            ecalHitCellH1->Fill(cell);

            // Check cell XY matches the position from the hit.
            std::pair<float,float> cellXY = ecalReadout.getCellCentroidXYPair(cell);
            std::cout << "cellXY = " << cellXY.first << ", " << cellXY.second << std::endl;
            if (cellXY.first != pos[0] || cellXY.second != pos[1]) {
                std::cout << "cellXY = " << cellXY.first << ", " << cellXY.second
                        << " does not match hit pos " << pos[0] << ", " << pos[1] << std::endl;
                ++cellXYbad;
            } else {
                ++cellXYmatch;
            }

            // Check cell ID computed by hex readout to see that it matches the decoded value.
            int foundCellID = ecalReadout.getCellId(pos[0], pos[1]);
            if (foundCellID != cell) {
                std::cout << "Found ECal cell ID " << foundCellID << " does not match " << cell << std::endl;
            }

            unsigned nContribs = calHit->getNumberOfContribs();
            std::cout << "nContribs = " << nContribs << std::endl;
            for (int iContrib = 0; iContrib < nContribs; iContrib++) {
                SimCalorimeterHit::Contrib contrib = calHit->getContrib(iContrib);
                ecalContribEdepH1->Fill(contrib.edep);
                std::cout << "contrib -> PDGID = " << contrib.pdgCode << ", edep = " << contrib.edep << ", time = " << contrib.time << std::endl;
                if (contrib.particle) {
                    contrib.particle->Print();
                } else {
                    std::cout << "contrib particle is null!!!" << std::endl;
                }
                std::cout << std::endl;
            }
            */

            //std::cout << std::endl;
        //}
    }

    histFile.Write();
    histFile.Close();

    return 0;
}

/*

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


}

*/
