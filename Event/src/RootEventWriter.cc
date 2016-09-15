#include "../include/Event/RootEventWriter.h"

RootEventWriter* RootEventWriter::instance = 0;

RootEventWriter::RootEventWriter(std::string theFileName) :
    fileName(theFileName),
    rootFile(0),
    tree(0),
    event(0) {
}

RootEventWriter::RootEventWriter() :
    fileName("sim_events.root"),
    rootFile(0),
    tree(0),
    event(0) {
}

RootEventWriter::~RootEventWriter() {
    // TODO: delete everything
}

RootEventWriter* RootEventWriter::getInstance() {
    if (instance == 0) {
        instance = new RootEventWriter();
    }
    return instance;
}

Event* RootEventWriter::getEvent() {
    return event;
}

void RootEventWriter::setFileName(std::string fileName) {
    this->fileName = fileName;
}

void RootEventWriter::open() {
    rootFile = new TFile(fileName.c_str(), "RECREATE");
    tree = new TTree("LDMX_Event", "LDMX event tree");
    event = new Event();
    tree->Branch("Event", "Event", &event, 32000, 3);
}

void RootEventWriter::write() {
    // Fill the tree from the event object.
    tree->Fill();

    // Write out data to the file.
    rootFile->Write();

    // Clear the current event object.
    event->Clear();
}

void RootEventWriter::close() {
    rootFile->Close();
}
