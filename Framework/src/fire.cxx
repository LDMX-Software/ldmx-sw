
//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

//-------------//
//   ldmx-sw   //
//-------------//
#include "Framework/Process.h"
#include "Framework/EventProcessorFactory.h"
#include "Framework/ConfigurePython.h"

/**
 * @namespace ldmx
 * @brief All classes in the ldmx-sw project use this namespace.
 */
using namespace ldmx;

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
 * @mainpage
 *
 * <a href="https://confluence.slac.stanford.edu/display/MME/Light+Dark+Matter+Experiment">LDMX</a> 
 * C++ Software Framework providing a full <a href="http://geant4.cern.ch">Geant4</a> simulation and 
 * flexible event processing and analysis chain.  The IO and analysis tools are based on 
 * <a href="http://root.cern.ch">ROOT</a>, and detectors are described in the 
 * <a href="https://gdml.web.cern.ch/GDML/">GDML</a> XML language.
 *
 * Refer to the <a href="https://github.com/LDMXAnalysis/ldmx-sw/">ldmx-sw github</a> for more information,
 * including build and usage instructions.
 */
int main(int argc, char* argv[]) {

    if (argc < 2) {
        printUsage();
        return 1;
    }

    int ptrpy = 1;
    for (ptrpy = 1; ptrpy < argc; ptrpy++) {
        if (strstr(argv[ptrpy], ".py"))
            break;
    }

    if (ptrpy == argc) {
        printUsage();
        std::cout << " ** No python configuration script provided (must end in '.py'). ** " << std::endl;
        return 1;
    }

    ProcessHandle p;
    std::cout << "---- LDMXSW: Loading configuration --------" << std::endl;
    try {
        
        ConfigurePython cfg(argv[ptrpy], argv + ptrpy + 1, argc - ptrpy - 1);
        p = cfg.makeProcess();
    } catch (Exception& e) {
        std::cerr << "Configuration Error [" << e.name() << "] : " << e.message() << std::endl;
        std::cerr << "  at " << e.module() << ":" << e.line() << " in " << e.function() << std::endl;
        return 1;
    }

    std::cout << "---- LDMXSW: Configuration load complete  --------" << std::endl;

    // If Ctrl-c is used, immediately exit the application.
    struct sigaction act;
    memset (&act, '\0', sizeof(act));
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror ("sigaction");
        return 1;
    }

    // See comment above for reason why this code is commented out.
    /* Use the sa_sigaction field because the handles has two additional parameters */
    //act.sa_sigaction = &softFinish;

    /* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
    //act.sa_flags = SA_SIGINFO;

    std::cout << "---- LDMXSW: Starting event processing --------" << std::endl;
    
    //process has its own try-catch branch for any exceptions
    p->run();
    
    std::cout << "---- LDMXSW: Event processing complete  --------" << std::endl;

    return 0;
}

void printUsage() {
    std::cout << "Usage: fire {configuration_script.py} [arguments to configuration script]" << std::endl;
    std::cout << "     configuration_script.py  (required) python script to configure the processing" << std::endl;
    std::cout << "     arguments                (optional) passed to configuration script when run in python" << std::endl;
}

