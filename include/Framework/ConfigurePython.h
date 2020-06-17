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
#include "Framework/Parameters.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <any>
#include <string>
#include <vector>

namespace ldmx {

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
             * The entire configuration for this process
             *
             * Contains all parameters that were passed in the python.
             */
            Parameters configuration_;

    };  // ConfigurePython

} // ldmx

#endif  // FRAMEWORK_CONFIGURE_PYTHON_H

