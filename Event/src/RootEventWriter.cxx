#include "Event/RootEventWriter.h"

// LDMX
#include "Event/EventConstants.h"

// STL
#include <iostream>

namespace event {

RootEventWriter::RootEventWriter(std::string fileName) :
    fileName(fileName),
    rootFile(0),
    tree(0),
    event(new Event()) {
}

RootEventWriter::RootEventWriter() :
    fileName("ldmx_events.root"),
    rootFile(0),
    tree(0),
    event(new Event()) {
}

RootEventWriter::~RootEventWriter() {
    delete rootFile;
}

Event* RootEventWriter::getEvent() {
    return event;
}

void RootEventWriter::setFileName(std::string fileName) {
    this->fileName = fileName;
}

void RootEventWriter::open() {

    std::cout << "Opening ROOT file " << fileName << " for writing" << std::endl;

    rootFile = new TFile(fileName.c_str(), "recreate");
    tree = new TTree("LDMX_Event", "LDMX event tree");
    tree->Branch("LdmxEvent" /* branch name */, "event::Event" /* class name */, &event, 32000, 3);
}

void RootEventWriter::writeEvent() {

    std::cout << std::endl;
    std::cout << "Writing event " << event->getEventNumber() << std::endl;
    std::cout << SIM_PARTICLES << ": " << event->getCollection(SIM_PARTICLES)->GetEntries() << std::endl;
    std::cout << RECOIL_SIM_HITS << ": " << event->getCollection(RECOIL_SIM_HITS)->GetEntries() << std::endl;
    std::cout << TAGGER_SIM_HITS << ": " << event->getCollection(TAGGER_SIM_HITS)->GetEntries() << std::endl;
    std::cout << ECAL_SIM_HITS << ": " << event->getCollection(ECAL_SIM_HITS)->GetEntries() << std::endl;
    std::cout << HCAL_SIM_HITS << ": " << event->getCollection(HCAL_SIM_HITS)->GetEntries() << std::endl;
    std::cout << std::endl;

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
