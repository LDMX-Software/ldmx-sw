#ifndef EVENT_ROOTWRITER_H_
#define EVENT_ROOTWRITER_H_

// ROOT
#include "TFile.h"
#include "TTree.h"

// LDMX
#include "Event/Event.h"

namespace event {

class RootEventWriter {

    public:

        RootEventWriter();

        RootEventWriter(std::string fileName, Event* outputEvent);

        RootEventWriter(Event* outputEvent);

        virtual ~RootEventWriter() {;}

        Event* getEvent() {
            return outputEvent_;
        }

        void setEvent(Event* outputEvent) {
            this->outputEvent_ = outputEvent;
        }

        void setFileName(std::string fileName) {
            this->fileName_ = fileName;
        }

        void open();

        void close();

        void writeEvent();

    private:

        std::string fileName_;
        TFile* rootFile_;
        TTree *tree_;
        Event* outputEvent_;
};

}

#endif
