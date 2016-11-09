#include "EventProc/RootEventSource.h"

// ROOT
#include "TFile.h"
#include "TTree.h"

// STL
#include <stdexcept>

namespace eventproc {

bool RootEventSource::openNextFile() {
    if (!fileList_.empty()) {
        std::string fileName = fileList_.front();
        fileList_.pop_front();
        if (file_ != nullptr) {
            delete file_;
        }
        file_ = new TFile(fileName.c_str());
        tree_ = (TTree*) file_->Get("LDMX_Event");
        branch_ = tree_->GetBranch("LdmxEvent");
        if (branch_ == NULL) {
            throw std::runtime_error("The LdmxEvent branch could not be read from the ROOT file.");
        }
        branch_->SetAddress(&_event);
        entry_ = 0;
        return true;
    } else {
        return false;
    }
}

bool RootEventSource::readNextEvent() {
    if (tree_ == nullptr || entry_ >= tree_->GetEntries()) {
        bool openedFile = openNextFile();
        if (!openedFile) {
            return false;
        }
    }
    tree_->GetEntry(entry_);
    ++entry_;
    return true;
}

}

