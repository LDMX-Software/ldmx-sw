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

    std::cout << "opening ROOT file " << fileName << " for writing" << std::endl;

    rootFile = new TFile(fileName.c_str(), "RECREATE");
    tree = new TTree("LDMX_Event", "LDMX event tree");
    event = new Event();
    tree->Branch("LdmxEvent" /* branch name */, "Event" /* class name */, &event, 32000, 3);
}

void RootEventWriter::writeEvent() {
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
