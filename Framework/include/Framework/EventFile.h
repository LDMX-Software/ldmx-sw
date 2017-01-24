/**
 * @file EventFile.h
 * @brief Class for managing a file of events
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_EVENTFILE_H_
#define EVENT_EVENTFILE_H_

// LDMX
#include "Framework/EventImpl.h"

// ROOT
#include "TTree.h"
#include "TFile.h"

// STL
#include <string>
#include <vector>

namespace ldmx {

/**
 * @class EventFile
 * @brief Manages a file of events
 */
class EventFile {

    public:

        /**
         * Class constructor to make a file with a custom tree name.
         * @param fileName The file name.
         * @param treeName The name of the tree containing event data.
         * @param isOutputFile True if this file is written out.
         * @param compressionLevel The compression level.
         */
        EventFile(const std::string& fileName, std::string treeName, bool isOutputFile = false, int compressionLevel = 9);
        
        /**
         * Class constructor to make a file with the default tree name.
         * @param fileName The file name.
         * @param isOutputFile True if this file is written out.
         * @param compressionLevel The compression level.
         */
        EventFile(const std::string& fileName, bool isOutputFile = false, int compressionLevel = 9);

        /**
         * Class constructor for cloning data from a "parent" file.
         * @param fileName The file name.
         * @param cloneParent Parent file for cloning data tree.
         * @param compressionLevel The compression level.
         */
        EventFile(const std::string& fileName, EventFile* cloneParent, int compressionLevel = 9);

        /**
         * Add a rule for dropping collections from the output.
         * @param rule The rule for dropping collections.
         *
         * @todo Need to document the string format.
         * @todo Need to verify that dropping objects works.
         */
        void addDrop(const std::string& rule);

        /**
         * Set an EventImpl object containing the event data to work with this file.
         * @param evt The EventImpl object with event data.
         */
        void setupEvent(EventImpl* evt);

        /**
         * Get the EventImpl object containing the event data.
         * @return The EventImpl object containing event data.
         */
        EventImpl* getEvent() { return event_; };

        /**
         * Prepare the next event.
         * @return If event was prepared/read successfully.
         */
        bool nextEvent();

        /**
         * Close the file, writing the tree to disk if creating an output file.
         */
        void close();

    private:

        /** The number of entries in the tree. */
        Long64_t entries_{-1};

        /** The current entry in the tree. */
        Long64_t ientry_{-1};

        /** The file name. */
        std::string fileName_;

        /** True if file is an output file being written to disk. */
        bool isOutputFile_;

        /** The backing TFile for this EventFile. */
        TFile* file_{nullptr};

        /** The tree with event data. */
        TTree* tree_{nullptr};

        /** A parent file containing event data. */
        EventFile* parent_{nullptr};

        /** The object containing the actual event data (trees and branches). */
        EventImpl* event_{nullptr};
};
}

#endif
