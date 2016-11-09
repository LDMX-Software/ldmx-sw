#include "Event/RootEventWriter.h"

// STL
#include <iostream>

namespace event {

RootEventWriter::RootEventWriter() :
        fileName_("ldmx_events.root"),
        rootFile_(nullptr),
        tree_(nullptr),
        outputEvent_(nullptr) {
}

RootEventWriter::RootEventWriter(std::string fileName, Event* outputEvent) :
    fileName_(fileName),
    rootFile_(nullptr),
    tree_(nullptr),
    outputEvent_(outputEvent) {
}

RootEventWriter::RootEventWriter(Event* outputEvent) :
    fileName_("ldmx_events.root"),
    rootFile_(nullptr),
    tree_(nullptr),
    outputEvent_(outputEvent) {
}

void RootEventWriter::open() {

    std::cout << "Opening ROOT file " << fileName_ << " for writing" << std::endl;

    rootFile_ = new TFile(fileName_.c_str(), "recreate");
    tree_ = new TTree("LDMX_Event", "LDMX event tree");
    tree_->Branch("LdmxEvent" /* branch name */, outputEvent_->getEventType() /* class name */, &outputEvent_, 32000, 3);
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
    tree_->Fill();
}

void RootEventWriter::close() {

    std::cout << "Closing file " << fileName_ << std::endl;

    // Write ROOT tree to disk.
    rootFile_->Write();

    // Close the file.
    rootFile_->Close();

    std::cout << "File " << fileName_ << " is closed!" << std::endl;
}

}
