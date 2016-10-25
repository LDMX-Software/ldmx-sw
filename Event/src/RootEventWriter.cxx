#include "Event/RootEventWriter.h"

// STL
#include <iostream>

namespace event {

RootEventWriter::RootEventWriter() :
        fileName("ldmx_events.root"),
        rootFile(nullptr),
        tree(nullptr),
        outputEvent(nullptr) {
}

RootEventWriter::RootEventWriter(std::string fileName, Event* outputEvent) :
    fileName(fileName),
    rootFile(nullptr),
    tree(nullptr),
    outputEvent(outputEvent) {
}

RootEventWriter::RootEventWriter(Event* outputEvent) :
    fileName("ldmx_events.root"),
    rootFile(nullptr),
    tree(nullptr),
    outputEvent(outputEvent) {
}

void RootEventWriter::open() {

    std::cout << "Opening ROOT file " << fileName << " for writing" << std::endl;

    rootFile = new TFile(fileName.c_str(), "recreate");
    tree = new TTree("LDMX_Event", "LDMX event tree");
    tree->Branch("LdmxEvent" /* branch name */, outputEvent->getEventType() /* class name */, &outputEvent, 32000, 3);
}

void RootEventWriter::writeEvent() {

    /*
    std::cout << std::endl;
    std::cout << "Writing event " << outputEvent->getEventNumber() << std::endl;
    std::cout << SIM_PARTICLES << ": " << outputEvent->getCollection(SIM_PARTICLES)->GetEntries() << std::endl;
    std::cout << RECOIL_SIM_HITS << ": " << outputEvent->getCollection(RECOIL_SIM_HITS)->GetEntries() << std::endl;
    std::cout << TAGGER_SIM_HITS << ": " << outputEvent->getCollection(TAGGER_SIM_HITS)->GetEntries() << std::endl;
    std::cout << ECAL_SIM_HITS << ": " << outputEvent->getCollection(ECAL_SIM_HITS)->GetEntries() << std::endl;
    std::cout << HCAL_SIM_HITS << ": " << outputEvent->getCollection(HCAL_SIM_HITS)->GetEntries() << std::endl;
    std::cout << std::endl;
    */

    // Fill the tree from the event object.
    tree->Fill();
}

void RootEventWriter::close() {

    std::cout << "Closing file " << fileName << std::endl;

    // Write ROOT tree to disk.
    rootFile->Write();

    // Close the file.
    rootFile->Close();

    std::cout << "File " << fileName << " is closed!" << std::endl;
}

}
