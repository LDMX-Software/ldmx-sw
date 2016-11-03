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
            return outputEvent;
        }

        void setEvent(Event* outputEvent) {
            this->outputEvent = outputEvent;
        }

        void setFileName(std::string fileName) {
            this->fileName = fileName;
        }

        void open();

        void close();

        void writeEvent();

    private:

        std::string fileName;
        TFile* rootFile;
        TTree *tree;
        Event* outputEvent;
};

}

#endif
