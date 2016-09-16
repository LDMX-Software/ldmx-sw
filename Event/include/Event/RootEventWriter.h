#ifndef EVENT_ROOTWRITER_H_
#define EVENT_ROOTWRITER_H_ 1

// ROOT
#include "TFile.h"
#include "TTree.h"

// LDMX
#include "Event/Event.h"

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

        int nWritten;
        int buffer;

        static RootEventWriter* INSTANCE;
};

#endif
