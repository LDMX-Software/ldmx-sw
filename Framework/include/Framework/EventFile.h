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
#include <map>

namespace ldmx {

    class RunHeader;

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
             * @param isSingleOutput boolean check if only one output file is being written to
             * @param compressionLevel The compression level.
             */
            EventFile(const std::string& fileName, EventFile* cloneParent, bool isSingleOutput = false, int compressionLevel = 9);

            /**
             * Class destructor.
             */
            virtual ~EventFile();

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
             * Change pointer to different parent file.
             * @param parent pointer to new parent file
             */
            void updateParent(EventFile* parent);

            /**
             * Get the EventImpl object containing the event data.
             * @return The EventImpl object containing event data.
             */
            EventImpl* getEvent() { return event_; };

            /**
             * Prepare the next event.
             * @return If event was prepared/read successfully.
             */
            bool nextEvent(bool storeCurrentEvent=true);

            /**
             * Close the file, writing the tree to disk if creating an output file.
             */
            void close();

            /**
             * Write the run header into a separate tree in the output file.
             * @param runHeader The run header to write into the output file.
             * @throw Exception if file is not writable.
             */
            void writeRunHeader(RunHeader* runHeader);

            /**
             * Get the RunHeader for a given run, if it exists in the input file.
             * @param runNumber The run number.
             * @return The RunHeader from the input file.
             * @throw Exception if there is no RunHeader in the file with the given run number.
             */
            const RunHeader& getRunHeader(int runNumber);

            const std::string& getFileName() {
                return fileName_;
            }

            const std::map<int, RunHeader*>& getRunMap() {
                return runMap_;
            }

        private:

            /**
             * Fill the internal map of run numbers to RunHeader objects from the input file.
             *
             * @note This is called automatically the first time nextEvent() is
             * activated.  If there are no run headers in the input file (e.g. for
             * a new simulation file) the run map will not be filled.
             */
            void createRunMap();

            /**
             * Copy run header tree from parent to output file.
             */
            void copyRunHeaders();

        private:

            /** The number of entries in the tree. */
            Long64_t entries_{-1};

            /** The current entry in the tree. */
            Long64_t ientry_{-1};

            /** The file name. */
            std::string fileName_;

            /** True if file is an output file being written to disk. */
            bool isOutputFile_;

            /** True if there is only one output file */
            bool isSingleOutput_;

            /** The backing TFile for this EventFile. */
            TFile* file_{nullptr};

            /** The tree with event data. */
            TTree* tree_{nullptr};

            /** A parent file containing event data. */
            EventFile* parent_{nullptr};

            /** The object containing the actual event data (trees and branches). */
            EventImpl* event_{nullptr};

            /** Pointer to run header from input file. */
            RunHeader* runHeader_{nullptr};

            /** Map of run numbers to RunHeader objects read from the input file. */
            std::map<int, RunHeader*> runMap_;
    };
}

#endif
