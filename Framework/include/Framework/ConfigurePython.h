/**
 * @file ConfigurePython.h
 * @brief Utility class that reads/executes a python script and creates a 
 *        Process object based on the input.
 * @author Jeremy Mans, University of Minnesota
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef FRAMEWORK_CONFIGUREPYTHON_H
#define FRAMEWORK_CONFIGUREPYTHON_H

/*~~~~~~~~~~~~*/
/*   python   */
/*~~~~~~~~~~~~*/
#include "Python.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Process.h"
#include "Framework/FrameworkDef.h" 

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <any>
#include <string>
#include <vector>

namespace ldmx {

    /**
     * @struct HistogramInfo
     * @brief Encapsulates the information required to create a histogram 
     */
    struct HistogramInfo { 
     
        /// Name of the histogram 
        std::string name_;

        /// X axis label
        std::string xLabel_;  

        /// The number of bins
        int bins_; 

        /// The minimum value of the histogram axis
        int xmin_;

        /// The maximum value of the histogram axis 
        int xmax_;

    };

    /**
     * @struct ProcessorClass
     * @brief Represents the configuration of an EventProcessor in the job.
     */
    struct ProcessorClass : Class {

        /// Histograms associated with this class    
        std::vector<HistogramInfo> histograms_; 

    };

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
             * @throw Exception if any necessary components of the python configuration
             * are missing. e.g. The Process class or the different members of 
             * the lastProcess object.
             *
             * The basic premis of this constructor is to execute the python
             * configuration script by loading it into python as a module.
             * Then, **after the script has been executed**, all of the parameters
             * for the Process and EventProcessors are gathered from python.
             * The fact that the script has been executed means that the user
             * can get up to a whole lot of nonsense that can help them
             * make their work more efficient.
             *
             * @param pythonScript Filename location of the python script.
             * @param args Commandline arguments to be passed to the python script.
             * @param nargs Number of commandline arguments.
             */
            ConfigurePython(const std::string& pythonScript, char* args[], int nargs);

            /**
             * Class destructor.
             *
             * Does nothing at the moment.
             */
            ~ConfigurePython() { }

            /**
             * Create a process object based on the python file information
             *
             * No python parsing actually happens in this function.
             * All of the parsing is done in the constructor, this just
             * copies that information into the Process object.
             */
            ProcessHandle makeProcess();

        private:

            /**
             * Extract parameter array from a python object.
             *
             * Assumes that the input PyObject is a python dictionary.
             *
             * Iterates through the dictionary and translates the objects inside
             * of it into the type-specified C++ equivalents, then puts these
             * objects into a STL map that can be passed to the Parameters class.
             *
             * @param dictionary Python dictionary class.
             * @return Mapping between parameter name and value. 
             */
            std::map< std::string, std::any > getParameters(PyObject* dictionary); 
            

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

            /**
             * Compression setting for the output files taken from python file.
             */
            int compressionSetting_;

            /** Histogram output file name */
            std::string histoOutFile_{""};

            /** The sequence of EventProcessor objects to be executed in order. */
            std::vector<ProcessorClass> sequence_;


    };  // ConfigurePython

} // ldmx

#endif  // FRAMEWORK_CONFIGURE_PYTHON_H

