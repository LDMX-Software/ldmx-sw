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
             * @param passname Processing pass label
             */
            Process(const std::string& passname);

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
             * Add an event processor to the linear sequence of processors to run in this job
             * @param evtproc EventProcessor (Producer, Analyzer) to add to the sequence
             */
            void addToSequence(EventProcessor* evtproc);

            /**
             * Add an input file name to the list.
             * @param filename Input ROOT event file name
             */
            void addFileToProcess(const std::string& filename);

            /**
             * Add a rule for keeping/dropping event products
             *
             * @note Rules have the format: "keep/drop [name]_[pass]"
             * Either or both of [name] and [pass] can be wildcards '*'.
             * To keep all branches, use "keep *".  Rules are processed in
             * order and can overrule each other.  Therefore, to keep only
             * the simTracks from the "sim" pass of processing : "drop
             * *_sim" then "keep simTracks_sim".
             *
             * @param rule
             */
            void addDropKeepRule(const std::string& rule);

            /**
             * Set a single output event file name
             * @param filenameOut Output ROOT event file name
             */
            void setOutputFileName(const std::string& filenameOut);

            /**
             * Add an output file name to the list.  There should either be the same number 
             * of output file names as input file names or just one output file name.
             * @param filenameOut Output ROOT event file name
             */
            void addOutputFileName(const std::string& filenameOut);

            /**
             * Set the compression setting to give to the output file(s).
             * @param set setting to give to the TFile as the compression
             */
            void setCompressionSetting(int set) { 
                compressionSetting_ = set;
            }

            /**
             * Set the name for a histogram file to contain histograms created by EventProcessor 
             * objects.  If this name is not set, any such histograms will be created in memory.
             * @param filenameHisto Output histogram ROOT file name
             */
            void setHistogramFileName(const std::string& filenameOut);


            /**
             * Set the run number to be used when initiating new events from the job
             * @param run Run number to use
             */
            void setRunNumber(int run) {
                runForGeneration_=run;
            }

            /**
             * Set the maximum number of events to process.  Processing will stop 
             * when either there are no more input events or when this number of events have been processed.
             * @param limit Maximum number of events to process.  -1 indicates no limit.
             */
            void setEventLimit(int limit=-1) {
                eventLimit_=limit;
            }

            /** 
             * Set the frequency with which event information is printed. 
             * @param logFrequency The frequency specied as number of events.
             */
            inline void setLogFrequency(int logFrequency) { logFrequency_ = logFrequency; }

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
            int eventLimit_{-1};
            
            /** The frequency with which event info is printed. */
            int logFrequency_{-1}; 

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
