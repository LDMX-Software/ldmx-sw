#include "Event/RootEventWriter.h"

// STL
#include <iostream>

RootEventWriter* RootEventWriter::INSTANCE = 0;

RootEventWriter::RootEventWriter(std::string theFileName) :
    fileName(theFileName),
    rootFile(0),
    tree(0),
    event(0),
    buffer(50),
    nWritten(0) {
}

RootEventWriter::RootEventWriter() :
    fileName("sim_events.root"),
    rootFile(0),
    tree(0),
    event(0),
    buffer(50),
    nWritten(0) {
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
    tree->Branch("Event", "Event", &event, 32000, 3);
}

void RootEventWriter::writeEvent() {

    std::cout << "writing event " << event->getHeader()->getEventNumber() << " to ROOT file " << fileName << std::endl;

    // Fill the tree from the event object.
    tree->Fill();

    // Clear the current event object.
    event->Clear();

    // Increment event counter.
    ++nWritten;

    if (nWritten % buffer == 0) {
        // Periodically flush events to disk based on buffer setting.
        event->Write();
    }
}

void RootEventWriter::close() {

    std::cout << "Flushing events to file " << fileName << std::endl;

    // Write out data to the file to flush it if necessary.
    rootFile->Write();

    std::cout << "Closing file " << fileName << std::endl;

    // Close the file.
    rootFile->Close();

    std::cout << "File " << fileName << " is closed!" << std::endl;
    std::cout << "Wrote " << nWritten << " events into " << fileName << std::endl;
}
