#ifndef FRAMEWORK_CONFIGUREPYTHON_H
#define FRAMEWORK_CONFIGUREPYTHON_H

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "Framework/Process.h"

namespace framework {

/**
 * @class ConfigurePython
 * @brief Utility class which reads/executes a python script and creates a
 *        Process object based on the input.
 * @note The configuration language is documented in the overall Framework
 *       package documentation.
 */
class ConfigurePython {
 public:
  /// the root configuration module name
  static std::string root_module;
  /// the root configuration class name
  static std::string root_class;
  /// the root configuration object name
  static std::string root_object;

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
   * The basic premise of this constructor is to execute the python
   * configuration script by loading it into python as a module.
   * Then, **after the script has been executed**, all of the parameters
   * for the Process and EventProcessors are gathered from python.
   * The fact that the script has been executed means that the user
   * can get up to a whole lot of shenanigans that can help them
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
  ~ConfigurePython() {}

  /**
   * Create a process object based on the python file information
   *
   * No python parsing actually happens in this function.
   * All of the parsing is done in the constructor, this just
   * copies that information from configuration_ into the Process object.
   *
   * @return ProcessHandle handle to process that is configured and ready to go
   */
  ProcessHandle makeProcess();

  /// Get a handle to the configuration
  const framework::config::Parameters get() const { return configuration_; }

 private:
  /**
   * The entire configuration for this process
   *
   * Contains all parameters that were passed in the python
   * in a recursive manner. For example, this would contain
   * a key called 'sequence' that has a value of a vector
   * of Parmaeters, each one corresponding to a processor.
   */
  framework::config::Parameters configuration_;

};  // ConfigurePython

}  // namespace framework

#endif  // FRAMEWORK_CONFIGURE_PYTHON_H
