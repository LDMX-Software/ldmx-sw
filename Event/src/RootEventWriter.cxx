#include "Event/RootEventWriter.h"

// STL
#include <iostream>

namespace event {

RootEventWriter::RootEventWriter(std::string fileName, Event* outputEvent) :
    fileName_(fileName), outputEvent_(outputEvent) {
}

void RootEventWriter::open() {

    // Create the file and tree.  They are cleaned up in the close() method.
    rootFile_ = new TFile(fileName_.c_str(), mode_.c_str(), "", compression_);
    tree_ = new TTree("LDMX_Event", "LDMX event tree");
    tree_->Branch("LdmxEvent" /* branch name */, outputEvent_->getEventType().c_str() /* class name */, &outputEvent_, 32000, 3);
}

void RootEventWriter::writeEvent() {

    // Fill the tree from the event object.
    tree_->Fill();

    // Clear the output event.
    outputEvent_->Clear("");
}

void RootEventWriter::close() {

    // Flush remaining data to file and cleanup.
    if (rootFile_) {
        rootFile_->Write();
        rootFile_->Close();
    }
}

void RootEventWriter::writeRunHeader(RunHeader* runHeader) {
    TTree* runTree = new TTree("LDMX_Run", "LDMX run header");
    TBranch* runBranch = runTree->Branch("LdmxRun", EventConstants::RUN_HEADER.c_str(), &runHeader, 32000, 3);
    runBranch->SetFile(rootFile_);
    runTree->Fill();
}

}
