#ifndef Event_RootWriter_h
#define Event_RootWriter_h

// ROOT
#include "TFile.h"
#include "TTree.h"

// LDMX
#include "Event/Event.h"

namespace event {

class RootEventWriter {

    public:

        RootEventWriter(std::string fileName);

        RootEventWriter();

        virtual ~RootEventWriter();

        static RootEventWriter* getInstance();

        void setFileName(std::string fileName);

        void open();

        void close();

        void writeEvent();

        Event* getEvent();

    private:

        std::string fileName;
        TFile* rootFile;
        TTree *tree;
        Event* event;

        static RootEventWriter* INSTANCE;
};

}

#endif
