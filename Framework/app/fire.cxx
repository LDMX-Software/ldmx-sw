
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
#include "Framework/Configure/Python.h"
#include "Framework/Process.h"

/**
 * @namespace framework
 * @brief All classes in the ldmx-sw project use this namespace.
 */
// using namespace framework;

// This code allows ldmx-app to exit gracefully when receiving preemption
// signals from the SDF (SIGUSR2) or Ctrl-c (SIGINT). It finishes the current
// event and closes ROOT files properly instead of losing work.
// NOTE: This seems to be container dependent. It has been tested and works
//       with apptainer v1.2 (at SDF) while does not work with podman.
// NOLINTNEXTLINE(readability-identifier-naming)
extern volatile std::sig_atomic_t preemption_received_;

static void softFinish(int sig, siginfo_t* siginfo, void* context) {
  preemption_received_ = 1;
}

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
    return 2;
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
    return 3;
  }

  std::cout << "---- LDMXSW: Loading configuration --------" << std::endl;

  framework::ProcessHandle p;
  try {
    framework::config::Parameters config{
        framework::config::run("ldmxcfg.Process.last_process", argv[ptrpy],
                               argv + ptrpy + 1, argc - ptrpy - 1)};
    p = std::make_unique<framework::Process>(config);
  } catch (const framework::exception::Exception& e) {
    // Error message currently printed twice since the stack trace code
    // sometimes crashes. Once this is fixed, the output above the stack trace
    // can be removed
    // https://github.com/LDMX-Software/Framework/issues/50
    std::cerr << "Configuration Error [" << e.name() << "] : " << e.message()
              << std::endl;
    std::cerr << "  at " << e.module() << ":" << e.line() << " in "
              << e.function() << std::endl;
    return 4;
  }

  std::cout << "---- LDMXSW: Configuration load complete  --------"
            << std::endl;

  // Setup signal handlers for graceful shutdown
  struct sigaction act;
  memset(&act, '\0', sizeof(act));
  act.sa_sigaction = &softFinish;
  act.sa_flags = SA_SIGINFO;

  // Handle SIGUSR2 (SDF preemption signal)
  if (sigaction(SIGUSR2, &act, NULL) < 0) {
    std::cerr << "Error setting up SIGUSR2 handler: " << strerror(errno)
              << std::endl;
    return 5;
  }

  // Also handle SIGINT (Ctrl-C)
  if (sigaction(SIGINT, &act, NULL) < 0) {
    std::cerr << "Error setting up SIGINT handler: " << strerror(errno)
              << std::endl;
    return 5;
  }

  std::cout << "---- LDMXSW: Starting event processing --------" << std::endl;

  try {
    p->run();
  } catch (const framework::exception::Exception& e) {
    // Process::run opens up the logging using the parameters passed to it from
    // python
    //  if an Exception is thrown, we haven't gotten to the end of Process::run
    //  where logging is closed, so we can do one more error message and then
    //  close it.
    // ldmx_log macro needs this variable to be named 'the_log_'
    // NOLINTNEXTLINE(readability-identifier-naming)
    auto the_log_{framework::logging::makeLogger("fire")};
    ldmx_log(fatal) << "[" << e.name() << "] : " << e.message() << "\n"
                    << "  at " << e.module() << ":" << e.line() << " in "
                    << e.function();
    ldmx_log(debug) << e.stackTrace();
    framework::logging::close();
    return 6;  // return non-zero error-status
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
