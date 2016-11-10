/**
 * @file RootEventWriter.h
 * @brief Class for writing output events using ROOT trees and branches.
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

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

        /** Open a writer with a file name and the Event object for the branch buffer. */
        RootEventWriter(std::string fileName, Event* outputEvent);

        virtual ~RootEventWriter() {;}
        
        /** Open the TFile and setup a TTree and event branch. */
        void open();

        /** 
         * Close the writer, performing cleanup.
         * Need to call open() to use the writer again.
        */
        void close();

        /** Fill the ROOT tree from the current event buffer. */
        void writeEvent();

        Event* getEvent() {
            return outputEvent_;
        }

        void setEvent(Event* outputEvent) {
            this->outputEvent_ = outputEvent;
        }
        
        const std::string& getFileName() const {
            return fileName_;
        }

        void setFileName(std::string fileName) {
            this->fileName_ = fileName;
        }
                       
        int getCompression() {
            return compression_;
        }
        
        void setCompression(int compression) {
            compression_ = compression;
        }
        
        const std::string& getMode() const {
            return mode_;
        }
                
        void setMode(std::string mode) {
            mode_ = mode;
        }
        
        TTree* getTree() const {
            return tree_;
        }
        
        TFile* getFile() const {
            return rootFile_;
        }
               
    private:

        std::string fileName_;
        Event* outputEvent_;
        TFile* rootFile_{nullptr};
        TTree* tree_{nullptr};
        int compression_{9};
        std::string mode_{"recreate"};
};

}

#endif
