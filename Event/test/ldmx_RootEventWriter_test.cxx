#include "Event/RootEventWriter.h"

#include "Event/EventConstants.h"
#include "Event/SimEvent.h"
#include "Event/SimCalorimeterHit.h"
#include "Event/SimTrackerHit.h"
#include "Event/SimCalorimeterHit.h"

using namespace event;

int main(int, const char* argv[])  {

    /*
     * Open a ROOT output file and add a dummy event.
     */

    SimEvent* outputEvent = new SimEvent();
    event::RootEventWriter writer("RootEventWriter_test.root", outputEvent);
    writer.open();

    outputEvent->setEventNumber(1234);
    outputEvent->setRun(42);
    outputEvent->setWeight(0.1);
    outputEvent->setTimestamp(12345678);

    SimTrackerHit* simTrackerHit = (SimTrackerHit*) outputEvent->getCollection(event::RECOIL_SIM_HITS)->ConstructedAt(0);
    SimCalorimeterHit* simCalorimeterHit = (SimCalorimeterHit*) outputEvent->getCollection(ECAL_SIM_HITS)->ConstructedAt(0);
    SimParticle* simParticle = (SimParticle*) outputEvent->getCollection(SIM_PARTICLES)->ConstructedAt(0);

    std::cout << "simTrackerHit: " << simTrackerHit << std::endl;
    std::cout << "simCalorimeterHit: " << simCalorimeterHit << std::endl;
    std::cout << "simParticle: " << simParticle << std::endl;

    //simTrackerHit->setSimParticle(simParticle);
    //simCalorimeterHit->setSimParticle(simParticle);

    writer.writeEvent();
    writer.close();

    /*
     * Reread the ROOT file that was written and dump out information.
     */

    TFile* file = TFile::Open("RootEventWriter_test.root", "READ");
    file->Print();
    TTree* tree = (TTree*) file->Get("LDMX_Event");
    tree->Print();
    SimEvent* inputEvent = new SimEvent();
    TBranch *branch = tree->GetBranch("LdmxEvent");
    if (branch == NULL) {
        std::cerr << "The LdmxEvent branch is null!" << std::endl;
        exit(1);
    }

    branch->SetAddress(&inputEvent);

    for(int entry = 0; entry < tree->GetEntries(); ++entry) {

        tree->GetEntry(entry);

        int eventNumber = inputEvent->getEventNumber();
        int nSimParticles = inputEvent->getCollection(event::SIM_PARTICLES)->GetEntries();
        int nTaggerHits = inputEvent->getCollection(event::TAGGER_SIM_HITS)->GetEntries();
        int nRecoilHits = inputEvent->getCollection(event::RECOIL_SIM_HITS)->GetEntries();
        int nEcalHits = inputEvent->getCollection(event::ECAL_SIM_HITS)->GetEntries();
        int nHcalHits = inputEvent->getCollection(event::HCAL_SIM_HITS)->GetEntries();

        std::cout << ">>> Event " << eventNumber << std::endl;
        std::cout << "  "  << event::SIM_PARTICLES << ": " << nSimParticles << std::endl;
        std::cout << "  "  << event::TAGGER_SIM_HITS << ": " << nTaggerHits << std::endl;
        std::cout << "  "  << event::RECOIL_SIM_HITS << ": " << nRecoilHits << std::endl;
        std::cout << "  "  << event::ECAL_SIM_HITS << ": " << nEcalHits << std::endl;
        std::cout << "  "  << event::HCAL_SIM_HITS << ": " << nHcalHits << std::endl;
        std::cout << std::endl;
    }

    file->Close();
}

