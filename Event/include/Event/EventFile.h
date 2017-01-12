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

        EventFile(const std::string& filename, std::string treeName, bool isOutputFile = false, int compressionLevel = 9);
        
        EventFile(const std::string& filename, bool isOutputFile = false, int compressionLevel = 9);

        EventFile(const std::string& filename, EventFile* cloneParent, int compressionLevel = 9);

        void addDrop(const std::string& rule);

        void setupEvent(EventImpl* evt);

        EventImpl* getEvent() { return event_; };

        bool nextEvent();

        void close();

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
