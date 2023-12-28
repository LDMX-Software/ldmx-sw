
//----------------//
//   C++ StdLib   //
//----------------//
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <iostream>

//-------------//
//   ldmx-sw   //
//-------------//
#include "Framework/ConfigurePython.h"
#include "Framework/Process.h"

/**
 * @namespace framework
 * @brief All classes in the ldmx-sw project use this namespace.
 */
// using namespace framework;

// This code allows ldmx-app to exit gracefully when Ctrl-c is used. It is
// currently causing segfaults when certain processors are used.  The code
// will remain commented out until these issues are investigated further.
/*
static Process* p { 0 };

static void softFinish (int sig, siginfo_t *siginfo, void *context) {
if (p) p->requestFinish();
}
*/

/**
 * @func printUsage
 *
 * Print how to use this executable to the terminal.
 */
void printUsage();

/**
 * @func fire main
 * @param[in] argc int number of command line arguments
 * @param[in] argv array of command line arguments
 *
 * We configure and run a framework::Process using the first command-line
 * argument ending in '.py' as the configu script for the framework::Process.
 * If no such argument is found, we error out.
 */
int main(int argc, char* argv[]) try {
  if (argc < 2) {
    printUsage();
    return 1;
  }

  int ptrpy = 1;
  for (ptrpy = 1; ptrpy < argc; ptrpy++) {
    if (strstr(argv[ptrpy], ".py")) break;
  }

  if (ptrpy == argc) {
    printUsage();
    std::cout << " ** No python configuration script provided (must end in "
                 "'.py'). ** "
              << std::endl;
    return 1;
  }

  std::cout << "---- LDMXSW: Loading configuration --------" << std::endl;

  framework::ProcessHandle p;
  try {
    framework::ConfigurePython cfg(argv[ptrpy], argv + ptrpy + 1,
                                   argc - ptrpy - 1);
    p = cfg.makeProcess();
  } catch (const framework::exception::Exception& e) {
    // Error message currently printed twice since the stack trace code
    // sometimes crashes. Once this is fixed, the output above the stack trace
    // can be removed
    // https://github.com/LDMX-Software/Framework/issues/50
    std::cerr << "Configuration Error [" << e.name() << "] : " << e.message()
              << std::endl;
    std::cerr << "  at " << e.module() << ":" << e.line() << " in "
              << e.function() << std::endl;
    return 1;
  }

  std::cout << "---- LDMXSW: Configuration load complete  --------"
            << std::endl;

  // If Ctrl-c is used, immediately exit the application.
  struct sigaction act;
  memset(&act, '\0', sizeof(act));
  if (sigaction(SIGINT, &act, NULL) < 0) {
    perror("sigaction");
    return 1;
  }

  // See comment above for reason why this code is commented out.
  /* Use the sa_sigaction field because the handles has two additional
   * parameters */
  // act.sa_sigaction = &softFinish;

  /* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not
   * sa_handler. */
  // act.sa_flags = SA_SIGINFO;

  std::cout << "---- LDMXSW: Starting event processing --------" << std::endl;

  try {
    p->run();
  } catch (const framework::exception::Exception& e) {
    // Process::run opens up the logging using the parameters passed to it from
    // python
    //  if an Exception is thrown, we haven't gotten to the end of Process::run
    //  where logging is closed, so we can do one more error message and then
    //  close it.
    auto theLog_{framework::logging::makeLogger(
        "fire")};  // ldmx_log macro needs this variable to be named 'theLog_'
    ldmx_log(fatal) << "[" << e.name() << "] : " << e.message() << "\n"
                    << "  at " << e.module() << ":" << e.line() << " in "
                    << e.function() << std::endl;
    framework::logging::close();
    return 1;  // return non-zero error-status
  }

  std::cout << "---- LDMXSW: Event processing complete  --------" << std::endl;
  return 0;
} catch (const std::exception& e) {
  std::cerr << "Unrecognized Exception: " << e.what() << std::endl;
  return 127;
}

void printUsage() {
  std::cout << "Usage: fire {configuration_script.py} [arguments to "
               "configuration script]"
            << std::endl;
  std::cout << "     configuration_script.py  (required) python script to "
               "configure the processing"
            << std::endl;
  std::cout << "     arguments                (optional) passed to "
               "configuration script when run in python"
            << std::endl;
}
