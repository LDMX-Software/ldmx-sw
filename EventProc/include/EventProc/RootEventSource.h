#ifndef EventProc_RootEventSource_h
#define EventProc_RootEventSource_h

// ROOT
#include "TTree.h"

// LDMX
#include "EventProc/EventSource.h"

// STL
#include <list>

namespace eventproc {

class RootEventSource : public EventSource {

    public:

        RootEventSource(std::list<std::string> fileList)
            : EventSource(),
              fileList(fileList),
              entry(0),
              tree(nullptr),
              file(nullptr),
              branch(nullptr) {
        }

        RootEventSource(std::string fileName)
            : EventSource(),
              entry(0),
              tree(nullptr),
              file(nullptr),
              branch(nullptr) {
            fileList.push_back(fileName);
        }

        virtual ~RootEventSource() {
        }

        bool readNextEvent();

    private:

        bool openNextFile();

    private:
        std::list<std::string> fileList;
        int entry;
        TTree* tree;
        TFile* file;
        TBranch *branch;
};

}

#endif
