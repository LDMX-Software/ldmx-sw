#ifndef FRAMEWORK_CONFIGUREPYTHON_H
#define FRAMEWORK_CONFIGUREPYTHON_H

#include "Framework/Configure/Parameters.h"

/**
 * python execution and parameter extraction
 *
 * this namespace is focused on holding the necessary
 * functions to run and extract the configuration parameters
 * from a python script. The documentation here is focused
 * on the C++-side of the configuration - i.e. the translation
 * of Python objects into their C++ counterparts.
 */
namespace framework::config {

/**
 * run the python script and extract the parameters
 *
 * This method contains all the parsing and execution of the python script.
 *
 * @throw Exception if the python script does not exit properly
 * @throw Exception if any necessary components of the python configuration
 * are missing. e.g. The Process class or the different members of
 * the lastProcess object.
 *
 * The basic premise of this function is to execute the python
 * configuration script. Then, **after the script has been executed**,
 * all of the parameters for the Process are gathered from python.
 * The fact that the script has been executed means that the user
 * can get up to a whole lot of shenanigans that can help them
 * make their work more efficient.
 *
 * @param[in] full pythonic path to the object to kickoff extraction
 * @param[in] pythonScript Filename location of the python script.
 * @param[in] args Commandline arguments to be passed to the python script.
 * @param[in] nargs Number of commandline arguments, assumed to be >= 0
 */
Parameters run(const std::string& root_object, const std::string& pythonScript,
               char* args[], int nargs);

}  // namespace framework::config

#endif  // FRAMEWORK_CONFIGURE_PYTHON_H
