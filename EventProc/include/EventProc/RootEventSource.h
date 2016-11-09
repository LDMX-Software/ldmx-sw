#ifndef EVENTPROC_ROOTEVENTSOURCE_H_
#define EVENTPROC_ROOTEVENTSOURCE_H_

// ROOT
#include "TTree.h"

// LDMX
#include "EventProc/EventSource.h"

// STL
#include <list>

namespace eventproc {

class RootEventSource : public EventSource {

    public:

        RootEventSource(std::list<std::string> fileList, Event* event)
            : EventSource(event),
              fileList_(fileList),
              entry_(0),
              tree_(nullptr),
              file_(nullptr),
              branch_(nullptr) {
        }

        RootEventSource(std::string fileName, Event* event)
            : EventSource(event),
              entry_(0),
              tree_(nullptr),
              file_(nullptr),
              branch_(nullptr) {
            fileList_.push_back(fileName);
        }

        virtual ~RootEventSource() {
        }

        bool readNextEvent();

    private:

        bool openNextFile();

    private:
        std::list<std::string> fileList_;
        int entry_;
        TTree* tree_;
        TFile* file_;
        TBranch *branch_;
};

}

#endif
