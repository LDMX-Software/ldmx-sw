#include "EventProc/RootEventSource.h"

// ROOT
#include "TFile.h"
#include "TTree.h"

// STL
#include <stdexcept>

namespace eventproc {

bool RootEventSource::openNextFile() {
    if (!fileList.empty()) {
        std::string fileName = fileList.front();
        fileList.pop_front();
        if (file != nullptr) {
            delete file;
        }
        file = new TFile(fileName.c_str());
        tree = (TTree*) file->Get("LDMX_Event");
        branch = tree->GetBranch("LdmxEvent");
        if (branch == NULL) {
            throw std::runtime_error("The LdmxEvent branch could not be read from the ROOT file.");
        }
        branch->SetAddress(&event);
        entry = 0;
        return true;
    } else {
        return false;
    }
}

bool RootEventSource::readNextEvent() {
    if (tree == nullptr || entry >= tree->GetEntries()) {
        bool openedFile = openNextFile();
        if (!openedFile) {
            return false;
        }
    }
    tree->GetEntry(entry);
    ++entry;
    return true;
}

}

