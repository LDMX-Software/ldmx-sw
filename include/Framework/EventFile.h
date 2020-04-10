/**
 * @file EventFile.h
 * @brief Class for managing a file of events
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_EVENTFILE_H_
#define EVENT_EVENTFILE_H_

// LDMX
#include "Framework/Event.h"

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
             *
             * This needs to be called *after* setupEvent.
             * This method uses the event to help drop collections.
             *
             * The rules should be of the following form:
             *      <drop,keep, or ignore> exp
             *      where exp is an expression that matches the collectionName.
             *      ignore ==> any branch with name matching exp is not even read in from the input file (if it exists)
             *      keep   ==> any branch with name matching exp is read in from the input (if exists) and written to output (if exists)
             *      drop   ==> any branch with name matching exp is read in from the input (if exists) and NOT written to output
             *
             * The default behavior for all branches is keep.
             *
             * ROOT uses the internal TRegexp to match branch names to the passed
             * expression and set the status of the branch (whether it will be read or not).
             * This internal object has different rules than real regular expressions, 
             * so it is best to keep it safe and only use asterisks or full names.
             * Additionally, the rules you pass are analyzed in succession, so you can go from something more general
             * to something more specific.
             *
             * For example to drop all EcalSimHits.*
             *      drop EcalSimHits.*
             *
             * or drop scoring plane collections
             *      drop .*ScoringPlane.*
             *
             * or drop scoring plane collections but keep EcalScoringPlane collection
             *      drop .*ScoringPlane.*
             *      keep EcalScoringPlane.*
             *
             * @note The Event::getImpl overrides any ignore rules for the input file in order to avoid any seg faults.
             * The items accessed will still be dropped.
             *
             * @param rule The rule for dropping collections.
             */
            void addDrop(const std::string& rule);

            /**
             * Set an Event object containing the event data to work with this file.
             * @param evt The Event object with event data.
             */
            void setupEvent(Event* evt);

            /**
             * Change pointer to different parent file.
             * @param parent pointer to new parent file
             */
            void updateParent(EventFile* parent);

            /**
             * Get the Event object containing the event data.
             * @return The Event object containing event data.
             */
            Event* getEvent() { return event_; };

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

            /**
             * Copy TTree addresses from new parent_ to our tree_.
             *
             * This is copied from TTree::CopyAddresses where edits
             * have been made to skip over Warnings that are expected.
             */
            void copyAddresses(TTree* parentTree);

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
            Event* event_{nullptr};

            /** Pointer to run header from input file. */
            RunHeader* runHeader_{nullptr};

            /**
             * Pre-clone rules.
             *
             * The series of rules to call before cloning/copying the
             * parent tree.
             */
            std::vector< std::pair< std::string , bool > > preCloneRules_;

            /** 
             * Vector of drop rules that have been parsed and
             * need to be used to reactivate these branches on the input tree
             *
             * The branches were initial deactivated so they don't get cloned 
             * to output tree.
             */
            std::vector< std::string > reactivateRules_;

            /** Map of run numbers to RunHeader objects read from the input file. */
            std::map<int, RunHeader*> runMap_;

            /// Time at which processing of events starts in seconds since epoch
            int processStart_{0}; 
    };
}

#endif
