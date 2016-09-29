#include "Event/RootEventWriter.h"

// STL
#include <iostream>

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
    tree->Branch("LdmxEvent" /* branch name */, "Event" /* class name */, &event, 32000, 3);
}

void RootEventWriter::writeEvent() {

    std::cout << std::endl;
    std::cout << "Writing event " << event->getHeader()->getEventNumber() << std::endl;
    std::cout << Event::SIM_PARTICLES << ": " << event->getCollectionSize(Event::SIM_PARTICLES) << std::endl;
    std::cout << Event::RECOIL_SIM_HITS << ": " << event->getCollectionSize(Event::RECOIL_SIM_HITS) << std::endl;
    std::cout << Event::TAGGER_SIM_HITS << ": " << event->getCollectionSize(Event::TAGGER_SIM_HITS) << std::endl;
    std::cout << Event::ECAL_SIM_HITS << ": " << event->getCollectionSize(Event::ECAL_SIM_HITS) << std::endl;
    std::cout << Event::HCAL_SIM_HITS << ": " << event->getCollectionSize(Event::HCAL_SIM_HITS) << std::endl;
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
