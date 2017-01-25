#include "Framework/EventFile.h"
#include "Framework/EventImpl.h"
#include "Framework/Exception.h"
#include "Event/EventConstants.h"

namespace ldmx {

EventFile::EventFile(const std::string& filename, std::string treeName, bool isOutputFile, int compressionLevel) :
        fileName_(filename), isOutputFile_(isOutputFile) {

    if (isOutputFile_) {
        file_ = new TFile(filename.c_str(), "RECREATE");
        if (!file_->IsWritable()) {
            EXCEPTION_RAISE("FileError", "Output file '" + filename + "' is not writable");
        }

    } else {
        file_ = new TFile(filename.c_str());
    }

    if (!file_->IsOpen()) {
        EXCEPTION_RAISE("FileError", "File '" + filename + "' is not readable or does not exist.");
    }

    if (isOutputFile_) {
        file_->SetCompressionLevel(compressionLevel);
    }

    if (!isOutputFile_) {
        tree_ = (TTree*) (file_->Get(treeName.c_str()));
        entries_ = tree_->GetEntriesFast();
    }
}

EventFile::EventFile(const std::string& filename, bool isOutputFile, int compressionLevel) :
        EventFile(filename, EventConstants::EVENT_TREE_NAME, isOutputFile, compressionLevel) {
}

EventFile::EventFile(const std::string& filename, EventFile* cloneParent, int compressionLevel) :

        fileName_(filename), isOutputFile_(true), parent_(cloneParent) {

    file_ = new TFile(filename.c_str(), "RECREATE");
    if (!file_->IsWritable()) {
        EXCEPTION_RAISE("FileError", "Output file '" + filename + "' is not writable");
    }

    if (!file_->IsOpen()) {
        EXCEPTION_RAISE("FileError", "File '" + filename + "' is not readable or does not exist.");
    }

    parent_->tree_->SetBranchStatus("*", 1);

    if (isOutputFile_) {
        file_->SetCompressionLevel(compressionLevel);
    }
}

void EventFile::addDrop(const std::string& rule) {

    if (parent_ == 0)
        return;

    size_t i = rule.find("keep");
    bool iskeep = i != std::string::npos;
    if (!iskeep)
        i = rule.find("drop");

    if (i == std::string::npos)
        return;

    std::string srule = rule.substr(i + 4);
    for (i = srule.find_first_of(" \t\n\r"); i != std::string::npos; i = srule.find_first_of(" \t\n\r"))
        srule.erase(i, 1);

    if (srule.length() == 0)
        return;

    if (srule.back() != '*')
        srule += '*';

    parent_->tree_->SetBranchStatus(srule.c_str(), (iskeep) ? (1) : (0));
}

bool EventFile::nextEvent() {

    if (ientry_ < 0 && parent_) {
        if (!parent_->tree_) {
            EXCEPTION_RAISE("EventFile", "No event tree in the file");
        }
        tree_ = parent_->tree_->CloneTree(0);
        event_->setInputTree(parent_->tree_);
        event_->setOutputTree(tree_);
    }

    // close up the last event
    if (ientry_ >= 0) {
        if (isOutputFile_) {
            event_->beforeFill();
            tree_->Fill(); // fill the clones...
            event_->Clear();
        }
        if (event_) {
            event_->onEndOfEvent();
        }
    }

    if (parent_) {
        if (!parent_->nextEvent()) {
            return false;
        }
        parent_->tree_->GetEntry(parent_->ientry_);
        ientry_ = parent_->ientry_;
        event_->nextEvent();
        entries_++;
        return true;

    } else {

        // if we are reading, move the pointer
        if (!isOutputFile_) {

            if (ientry_ + 1 >= entries_) {
                return false;
            }

            ientry_++;
            tree_->LoadTree(ientry_);

            if (event_) {
                event_->nextEvent();
            }
            return true;

        } else {
            ientry_++;
            entries_++;
            return true;
        }
    }
    return false;
}

void EventFile::setupEvent(EventImpl* evt) {
    event_ = evt;
    if (isOutputFile_) {
        if (!tree_ && !parent_) {
            tree_ = event_->createTree();
            ientry_ = 0;
            entries_ = 0;
        }

        if (parent_) {
            event_->setInputTree(parent_->tree_);
        }

    } else {
        event_->setInputTree(tree_);
    }
}

void EventFile::close() {
    if (isOutputFile_)
        tree_->Write();
    file_->Close();
}

void EventFile::writeRunHeader(RunHeader* runHeader) {

    // Throw an error if file is not writable.
    if (!isOutputFile_) {
        EXCEPTION_RAISE("FileError", "Output file '" + fileName_ + "' is not writable.");
    }

    // Throw an error if tree already exists in output file.
    TTree* runTree = (TTree*) file_->Get("LDMX_Run");
    if (runTree) {
        EXCEPTION_RAISE("DataError", "The run header already exists in the output file.");
    }

    // Create the tree and branch for the RunHeader, fill it, and write out the tree.
    runTree = new TTree("LDMX_Run", "LDMX run header");
    TBranch* runBranch = runTree->Branch("RunHeader", EventConstants::RUN_HEADER.c_str(), &runHeader, 32000, 3);
    runBranch->SetFile(file_);
    runTree->Fill();
    runTree->Write();
}

const RunHeader& EventFile::getRunHeader() {
    // Read the run header from the separate tree in the input file if it was not read already.
    if (!runHeader_) {
        TTree* runTree = (TTree*) file_->Get("LDMX_Run");
        if (!runTree) {
            EXCEPTION_RAISE("DataError", "The run header does not exist in the input file.");
        }
        runTree->SetBranchAddress("RunHeader", &runHeader_);
        runTree->GetEntry(0);
    }
    return *runHeader_;
}

}
