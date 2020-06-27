/**
 * @file Process.h
 * @brief Class which represents the process under execution.
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef LDMXSW_FRAMEWORK_PROCESS_H_
#define LDMXSW_FRAMEWORK_PROCESS_H_

// LDMX
#include "Exception/Exception.h"
#include "Framework/StorageControl.h"
#include "Framework/Parameters.h"

// STL
#include <vector>
#include <memory>

class TFile;
class TDirectory;

namespace ldmx {

    class EventProcessor;
    class EventFile;
    class Event;

    /**
     * @class Process
     * @brief Class which represents the process under execution.
     */
    class Process {

        public:

            /**
             * Class constructor.
             * @param configuration Parameters to configure process with
             */
            Process(const Parameters& configuration);

            /**
             * Class Destructor
             *
             * Cleans up sequence of EventProcessors.
             * These processors were created by ConfigurePython and should be deleted.
             */
            ~Process();

            /**
             * Get the processing pass label.
             * @return The processing pass label.
             */
            const std::string& getPassName() const {
                return passname_;
            }

            /**
             * Set the run number to be used when initiating new events from the job
             * @param run Run number to use
             */
            void setRunNumber(int run) {
                runForGeneration_=run;
            }

            /**
             * Get the frequency with which the event information is printed.
             * @return integer log frequency (negative if turned off)
             */
            int getLogFrequency() const { return logFrequency_; }

            /**
             * Run the process.
             */
            void run();

            /**
             * Request that the processing finish with this event
             */ 
            void requestFinish() { eventLimit_=0; }

            /**
             * Construct a TDirectory* for the given module
             */
            TDirectory* makeHistoDirectory(const std::string& dirName);

            /**
             * Open a ROOT TFile to write histograms and TTrees.
             */
            TDirectory* openHistoFile(); 

            /**  
             * Access the storage control unit for this process
             */
            StorageControl& getStorageController() { return m_storageController; }
    
        private:

            /** Processing pass name. */
            std::string passname_;

            /** Limit on events to process. */
            int eventLimit_;
            
            /** The frequency with which event info is printed. */
            int logFrequency_; 

            /** Maximum number of attempts to make before giving up on an event */
            int maxTries_;

            /** Storage controller */
            StorageControl m_storageController;

            /** Ordered list of EventProcessors to execute. */
            std::vector<EventProcessor*> sequence_;

            /** List of input files to process.  May be empty if this Process will generate new events. */
            std::vector<std::string> inputFiles_;

            /** List of output file names.  If empty, no output file will be created. */
            std::vector<std::string> outputFiles_;

            /** Compression setting to pass to output files
             *
             * Look at the documentation for the TFile constructor if you
             * want to learn more details. Essentially,
             * setting = 100*algo + level
             * with algo = 0 being the global default.
             */
            int compressionSetting_;

            /** Set of drop/keep rules. */
            std::vector<std::string> dropKeepRules_;

            /** Run number to use if generating events. */
            int runForGeneration_{1};

            /** Filename for histograms and other user products */
            std::string histoFilename_;

            /** TFile for histograms and other user products */
            TFile* histoTFile_{0};
    };

    /**
     * A handle to the current process
     * Used to pass a process from ConfigurePython
     * to ldmx-app.
     */
    typedef std::unique_ptr<Process> ProcessHandle;
}

#endif
