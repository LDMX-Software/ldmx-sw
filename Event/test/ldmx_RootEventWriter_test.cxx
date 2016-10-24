#include "Event/RootEventWriter.h"

#include "Event/EventConstants.h"
#include "Event/SimCalorimeterHit.h"
#include "Event/SimTrackerHit.h"
#include "Event/SimCalorimeterHit.h"

using namespace event;

int main(int, const char* argv[])  {

    event::RootEventWriter writer;
    writer.setFileName("RootEventWriter_test.root");

    writer.open();

    Event* theEvent = writer.getEvent();

    theEvent->setEventNumber(1234);
    theEvent->setRun(42);
    theEvent->setWeight(0.1);
    theEvent->setTimestamp(12345678);

    //std::cout << "event type: " << theEvent->getEventType() << std::endl;

    SimTrackerHit* simTrackerHit = (SimTrackerHit*) theEvent->getCollection(event::RECOIL_SIM_HITS)->ConstructedAt(0);
    SimCalorimeterHit* simCalorimeterHit = (SimCalorimeterHit*) theEvent->getCollection(ECAL_SIM_HITS)->ConstructedAt(0);
    SimParticle* simParticle = (SimParticle*) theEvent->getCollection(SIM_PARTICLES)->ConstructedAt(0);
    simTrackerHit->setSimParticle(simParticle);
    simCalorimeterHit->setSimParticle(simParticle);

    writer.writeEvent();

    writer.close();

    TFile* file = TFile::Open("RootEventWriter_test.root", "READ");
    file->Print();
    TTree* tree = (TTree*) file->Get("LDMX_Event");
    tree->Print();

    /*
    Event* event = new Event();
    TBranch *branch = tree->GetBranch("LdmxEvent");
    if (branch == NULL) {
        std::cerr << "The LdmxEvent branch is null!" << std::endl;
        exit(1);
    }
    */

    file->Close();
}

