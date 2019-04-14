/**
 * @file ConfigurePython.h
 * @brief Utility class that reads/executes a python script and creates a Process object based on the input.
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef FRAMEWORK_CONFIGUREPYTHON_H_
#define FRAMEWORK_CONFIGUREPYTHON_H_

//-------------//
//   ldmx-sw   //
//-------------//
#include "Framework/ParameterSet.h"

//----------------//
//   C++ StdLib   //
//----------------//
#include <string>
#include <vector>

namespace ldmx {

    class Process;

    /**
     * @class ConfigurePython
     * @brief Utility class which reads/executes a python script and creates a 
     *        Process object based on the input.
     * @note The configuration language is documented in the overall Framework 
     *       package documentation.
     */
    class ConfigurePython {

        public:

            /**
             * Class constructor.
             *
             * This method contains all the parsing and execution of the python script.
             *
             * @param pythonScript Filename location of the python script.
             * @param args Commandline arguments to be passed to the python script.
             * @param nargs Number of commandline arguments.
             */
            ConfigurePython(const std::string& pythonScript, char* args[], int nargs);

            /**
             * Class destructor.
             */
            ~ConfigurePython();

            /**
             * Create a process object based on the python file information
             */
            Process* makeProcess();

        private:

            /** The label for this processing pass. */
            std::string passname_;

            /** 
             * The maximum number of events to process, if provided in python
             * file. 
             */
            int eventLimit_ {-1};

            /** 
             * The run number to use when generating events (no input file), 
             * if provided in python file. 
             */
            int run_ {-1};

            /** The frequency with which event info is printed. */
            int logFrequency_{-1}; 

            /** 
             * List of input ROOT files to process in the job, if provided in 
             * python file. 
             */
            std::vector<std::string> inputFiles_;

            /** 
             * List of rules for keeping and dropping data products, if 
             * provided in python file. 
             */
            std::vector<std::string> keepRules_;

            /** Default sense for keeping events (keep or drop) */
            bool skimDefaultIsKeep_;

            /** 
             * List of rules for keeping and dropping events, if provided in 
             * python file. 
             */
            std::vector<std::string> skimRules_;

            /** 
             * List of rules for shared libraries to load, if provided in 
             * python file. 
             */
            std::vector<std::string> libraries_;

            /** 
             * List of rules for output ROOT file names, if provided in python
             * file. 
             */
            std::vector<std::string> outputFiles_;

            /** Histogram output file name */
            std::string histoOutFile_{""};

            struct HistogramInfo { 
                std::string name_;
                std::string xLabel_;  
                int bins_; 
                int xmin_; 
                int xmax_;
            };

            /**
             * @struct ProcessorInfo
             * @brief Represents the configuration of an EventProcessor in the job.
             */
            struct ProcessorInfo {
                    std::string classname_;
                    std::string instancename_;
                    ParameterSet params_;
                    std::vector<HistogramInfo> histograms_; 
            };

            /** The sequence of EventProcessor objects to be executed in order. */
            std::vector<ProcessorInfo> sequence_;
    };

}

#endif

