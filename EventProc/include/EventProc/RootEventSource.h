/**
 * @file RootEventSource.h
 * @brief Class that supplies ROOT format events to an EventLoop
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENTPROC_ROOTEVENTSOURCE_H_
#define EVENTPROC_ROOTEVENTSOURCE_H_

// ROOT
#include "TTree.h"

// LDMX
#include "EventProc/EventSource.h"

// STL
#include <list>

namespace eventproc {

/**
 * @class RootEventSource
 * @brief Supplies ROOT format events to an event loop
 */
class RootEventSource : public EventSource {

    public:

        /**
         * Class constructor.
         * @param fileList The list of input file paths.
         * @param event The event buffer filled from the tree.
         */
        RootEventSource(std::list<std::string> fileList, Event* event)
            : EventSource(event),
              fileList_(fileList),
              entry_(0),
              tree_(nullptr),
              file_(nullptr),
              branch_(nullptr) {
        }

        /**
         * Class constructor.
         * @param fileName The path of an input file.
         * @param event The event buffer filled from the tree.
         */
        RootEventSource(std::string fileName, Event* event)
            : EventSource(event),
              entry_(0),
              tree_(nullptr),
              file_(nullptr),
              branch_(nullptr) {
            fileList_.push_back(fileName);
        }

        /**
         * Class destructor.
         */
        virtual ~RootEventSource() {
        }

        /**
         * Read the next event.
         * @return True if event was read successfully.
         */
        bool readNextEvent();

    private:

        /**
         * Open the next file in the list.
         * @return True if file was opened successfully.
         */
        bool openNextFile();

    private:

        /**
         * The list of files.
         */
        std::list<std::string> fileList_;

        /**
         * The current branch entry.
         */
        int entry_;

        /**
         * The current <i>TTree</i> from the file.
         */
        TTree* tree_;

        /**
         * The current open ROOT file.
         */
        TFile* file_;

        /**
         * The current branch in the file.
         */
        TBranch *branch_;
};

}

#endif
