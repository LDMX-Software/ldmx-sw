#include "Event/RootEventWriter.h"

// STL
#include <iostream>

namespace event {

RootEventWriter* RootEventWriter::INSTANCE = 0;

RootEventWriter::RootEventWriter(std::string fileName) :
    fileName(fileName),
    rootFile(0),
    tree(0),
    event(0) {
}

RootEventWriter::RootEventWriter() :
    fileName("ldmx_events.root"),
    rootFile(0),
    tree(0),
    event(0) {
}

RootEventWriter::~RootEventWriter() {
    delete rootFile;
}

RootEventWriter* RootEventWriter::getInstance() {
    if (INSTANCE == 0) {
        INSTANCE = new RootEventWriter();
    }
    return INSTANCE;
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
    event = new Event();
    tree->Branch("LdmxEvent" /* branch name */, "event::Event" /* class name */, &event, 32000, 3);
}

void RootEventWriter::writeEvent() {

    std::cout << std::endl;
    std::cout << "Writing event " << event->getEventNumber() << std::endl;
    std::cout << Event::SIM_PARTICLES << ": " << event->getCollection(Event::SIM_PARTICLES)->GetEntries() << std::endl;
    std::cout << Event::RECOIL_SIM_HITS << ": " << event->getCollection(Event::RECOIL_SIM_HITS)->GetEntries() << std::endl;
    std::cout << Event::TAGGER_SIM_HITS << ": " << event->getCollection(Event::TAGGER_SIM_HITS)->GetEntries() << std::endl;
    std::cout << Event::ECAL_SIM_HITS << ": " << event->getCollection(Event::ECAL_SIM_HITS)->GetEntries() << std::endl;
    std::cout << Event::HCAL_SIM_HITS << ": " << event->getCollection(Event::HCAL_SIM_HITS)->GetEntries() << std::endl;
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
