#include "Event/Event.h"
#include "Event/SimEvent.h"
#include "Event/SimTrackerHit.h"
#include "Event/SimCalorimeterHit.h"
#include "Event/SimParticle.h"

// ROOT
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"

#include <ctime>

using event::Event;
using event::EventConstants;
using event::SimEvent;
using event::SimTrackerHit;
using event::SimCalorimeterHit;
using event::SimParticle;

int main(int argc, char* argv[]) {

    /*
     * Write out a single dummy event to file.
     */

    SimEvent* outputEvent = new SimEvent();

    auto outputFile = new TFile("SimEvent_test.root", "recreate");
    auto outputTree = new TTree("LDMX_Event", "LDMX event tree");
    outputTree->Branch("LdmxEvent" /* branch name */,
            outputEvent->getEventType().c_str() /* class name */,
            &outputEvent,
            32000,
            3);

    outputEvent->setEventNumber(1234);
    outputEvent->setRun(42);
    outputEvent->setWeight(1.2);
    outputEvent->setTimestamp(std::time(0));

    SimTrackerHit* simTrackerHit = (SimTrackerHit*) outputEvent->getCollection(EventConstants::RECOIL_SIM_HITS)->ConstructedAt(0);
    SimCalorimeterHit* simCalorimeterHit = (SimCalorimeterHit*) outputEvent->getCollection(EventConstants::ECAL_SIM_HITS)->ConstructedAt(0);
    SimParticle* simParticle = (SimParticle*) outputEvent->getCollection(EventConstants::SIM_PARTICLES)->ConstructedAt(0);

    simTrackerHit->setSimParticle(simParticle);
    //simCalorimeterHit->setSimParticle(simParticle);

    outputTree->Fill();
    outputFile->Write();
    outputFile->Close();

    /*
     * Reread the ROOT file and dump out information.
     */

    TFile* file = TFile::Open("SimEvent_test.root", "READ");
    TTree* tree = (TTree*) file->Get("LDMX_Event");
    SimEvent* inputEvent = new SimEvent();
    TBranch *branch = tree->GetBranch("LdmxEvent");
    if (branch == NULL) {
        std::cerr << "The LdmxEvent branch is null!" << std::endl;
        exit(1);
    }

    branch->SetAddress(&inputEvent);

    const Event::CollectionMap& collMap = inputEvent->getCollectionMap();

    // event loop
    for (int entry = 0; entry < tree->GetEntries(); ++entry) {

        tree->GetEntry(entry);

        int eventNumber = inputEvent->getEventNumber();
        int nSimParticles = inputEvent->getCollection(EventConstants::SIM_PARTICLES)->GetEntries();
        int nTaggerHits = inputEvent->getCollection(EventConstants::TAGGER_SIM_HITS)->GetEntries();
        int nRecoilHits = inputEvent->getCollection(EventConstants::RECOIL_SIM_HITS)->GetEntries();
        int nEcalHits = inputEvent->getCollection(EventConstants::ECAL_SIM_HITS)->GetEntries();
        int nHcalHits = inputEvent->getCollection(EventConstants::HCAL_SIM_HITS)->GetEntries();

        std::cout << "Event " << eventNumber
                << ", run: " << inputEvent->getRun()
                << ", timestamp: " << inputEvent->getTimestamp()
                << ", weight: " << inputEvent->getWeight()
                << std::endl;
        std::cout << "  " << EventConstants::SIM_PARTICLES << ": " << nSimParticles << std::endl;
        std::cout << "  " << EventConstants::TAGGER_SIM_HITS << ": " << nTaggerHits << std::endl;
        std::cout << "  " << EventConstants::RECOIL_SIM_HITS << ": " << nRecoilHits << std::endl;
        std::cout << "  " << EventConstants::ECAL_SIM_HITS << ": " << nEcalHits << std::endl;
        std::cout << "  " << EventConstants::HCAL_SIM_HITS << ": " << nHcalHits << std::endl;
        std::cout << std::endl;

        // collection loop
        for (Event::CollectionMap::const_iterator iColl = collMap.begin();
                iColl != collMap.end(); iColl++) {
            TClonesArray* coll = (*iColl).second;
            // object loop
            for (int iObj = 0; iObj < coll->GetEntries(); iObj++) {
                TObject* obj = coll->At(iObj);
                obj->Print();
                /*
                 * Print out SimParticle referenced by hits.
                 */
                if (dynamic_cast<SimTrackerHit*>(obj)) {
                    SimTrackerHit* simTrackerHit = (SimTrackerHit*) obj;
                    SimParticle* simParticle = simTrackerHit->getSimParticle();
                    if (simParticle) {
                        simParticle->Print();
                    } else {
                        std::runtime_error("The SimParticle from SimTrackerHit is null!");
                    }
                } else if (dynamic_cast<SimCalorimeterHit*>(obj)) {
                    SimCalorimeterHit* simCalorimeterHit = (SimCalorimeterHit*) obj;
                    //SimParticle* simParticle = simCalorimeterHit->getSimParticle();
                    if (simParticle) {
                        simParticle->Print();
                    } else {
                        std::runtime_error("The SimParticle from SimCalorimeterHit is null!");
                    }
                }
            }
        }
    }

    file->Close();
}
