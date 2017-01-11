#ifndef EVENT_EVENTFILE_H_
#define EVENT_EVENTFILE_H_

// ROOT
#include "TTree.h"
#include "TFile.h"

// STL
#include <string>
#include <vector>

namespace event {

class EventImpl;

class EventFile {

    public:

        EventFile(const std::string& filename, bool isOutputFile = false);

        EventFile(const std::string& filename, EventFile* cloneParent);

        void addDrop(const std::string& rule);

        // could be source of the event if we have a place to get the pass name from
        void setupEvent(EventImpl* evt);

        bool nextEvent();

        void Close();

    private:

        Long64_t entries_{-1};
        Long64_t ientry_ {-1};

        std::string fileName_;

        bool isOutputFile_;

        TFile* file_{nullptr};
        TTree* tree_{nullptr};

        EventFile* parent_{nullptr};
        EventImpl* event_{nullptr};
};
}

#endif
