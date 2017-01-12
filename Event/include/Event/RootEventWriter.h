/**
 * @file RootEventWriter.h
 * @brief Class for writing Event objects to a ROOT tree using trees and branches
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_ROOTWRITER_H_
#define EVENT_ROOTWRITER_H_

// ROOT
#include "TFile.h"
#include "TTree.h"

// LDMX
#include "Event/Event.h"
#include "Event/RunHeader.h"

namespace event {

/**
 * @class RootEventWriter
 * @brief Writes ROOT data from an Event buffer into a file using trees and branches
 */
class RootEventWriter {

    public:

        /**
         * Class constructor.
         * @param fileName The name of the output file.
         * @param outputEvent The event object to be used as a branch buffer.
         */
        RootEventWriter(std::string fileName, Event* outputEvent);

        /**
         * Class destructor.
         */
        virtual ~RootEventWriter() {;}
        
        /**
         * Open the TFile and setup a new branch and tree for the output.
         */
        void open();

        /** 
         * Close the writer, performing cleanup.
         * To use the writer again, the <i>open()</i> method should be called.
        */
        void close();

        /**
         * Fill the ROOT tree from the current event and then clear it.
         */
        void writeEvent();

        /**
         * Get a pointer to the output event.
         * @return Pointer to the output event.
         */
        Event* getEvent() {
            return outputEvent_;
        }

        /**
         * Set the object which will be used as the event branch buffer.
         * @param outputEvent The event to be used as the output buffer.
         */
        void setEvent(Event* outputEvent) {
            this->outputEvent_ = outputEvent;
        }
        
        /**
         * Get the output file name.
         * @return The output file name.
         */
        const std::string& getFileName() const {
            return fileName_;
        }

        /**
         * Set the file name.
         * This will have no effect if the file is already open for writing.
         * @param fileName The output file name.
         */
        void setFileName(std::string fileName) {
            this->fileName_ = fileName;
        }
                       
        /**
         * Get the ROOT compression level.
         * @return The ROOT compression level.
         */
        int getCompression() {
            return compression_;
        }
        
        /**
         * Set the ROOT compression level (1-9).
         * @param compression The compression level.
         */
        void setCompression(int compression) {
            compression_ = compression;
        }
        
        /**
         * Get the write mode.
         * @return The write mode.
         */
        const std::string& getMode() const {
            return mode_;
        }
                
        /**
         * Set the write mode (NEW, RECREATE or UPDATE).
         * @mode The write mode.
         */
        void setMode(std::string mode) {
            mode_ = mode;
        }
        
        /**
         * Get the output ROOT tree.
         * @return The output ROOT tree.
         */
        TTree* getTree() const {
            return tree_;
        }
        
        /**
         * Get the output ROOT file.
         * @return The output ROOT file.
         */
        TFile* getFile() const {
            return rootFile_;
        }

        /**
         * Write a run header to a separate branch in the output file.
         * @param runHeader The run header to write out.
         */
        void writeRunHeader(RunHeader* runHeader);
               
    private:

        /**
         * The output file name.
         */
        std::string fileName_;

        /**
         * The output event.
         */
        Event* outputEvent_;

        /**
         * The output ROOT file.
         */
        TFile* rootFile_{nullptr};

        /**
         * The output ROOT tree.
         */
        TTree* tree_{nullptr};

        /**
         * The compression level.
         */
        int compression_{9};

        /**
         * The file creation mode.
         */
        std::string mode_{"recreate"};
};

}

#endif
