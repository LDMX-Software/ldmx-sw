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
#include "Exception/Logger.h"

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
 * @func printUsage()
 * Print usage for this application
 *
 * This function uses printf and _not_ the logger
 * because it is run before the logger is configured.
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

    int fileVerbosity = 0; //default to off
    int termVerbosity = 1; //default to warnings/errors only
    /**
     * Scan for python file
     *
     * While we are looking for it, check for other application arguments
     * Need to know what level to log for the terminal and for the file
     */
    int ptrpy = 1;
    for (ptrpy = 1; ptrpy < argc; ptrpy++) {
        std::string currArg(argv[ptrpy]);
        if (strstr(argv[ptrpy], ".py")) {
            //this argument is python file
            //  Assume rest of arguments are intended for python file
            break;
        } else if ( currArg == "-h" or currArg == "--help" ) {
            //print Usage message
            printUsage();
            return 0;
        } else if ( currArg == "-f" or currArg == "--fileLog" ) {
            //print ldmx-sw logging messages to a file
            if ( ptrpy+1 < argc and isdigit(argv[ptrpy+1][0]) ) {
                fileVerbosity = atoi( argv[ptrpy+1] );
            } else {
                fileVerbosity = 3; //maximum
            }
        } else if ( currArg == "-v" or currArg == "--verbosity" ) {
            //check for next arg being verbosity integer
            if ( ptrpy+1 < argc and isdigit(argv[ptrpy+1][0]) ) {
                termVerbosity = atoi( argv[ptrpy+1] );
            } else {
                termVerbosity = 3; //maximum
            }
        } 
    }

    /**
     * Got to end of arguments list without finding python file
     * Error out
     */
    if (ptrpy == argc) {
        printUsage();
        printf("\t** No python script provided. **\n");
        return 1;
    }

    //open up the log
    logging::open( 3 , 3 ); //input actual parameters

    ldmx::logging::logger lg;

    Process* p { 0 };
    try {
        BOOST_LOG(lg) << "----- LDMXSW: Loading configuration file -----"
        
        ConfigurePython cfg(argv[ptrpy], argv + ptrpy + 1, argc - ptrpy);
        p = cfg.makeProcess();

        std::cout << "----- LDMXSW: Configuration load complete -----"

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

        std::cout << "----- LDMXSW: Starting event processing -----" << std::endl;
        
        p->run();
        
        std::cout << "----- LDMXSW: Event processing complete -----" << std::endl;

    } catch (Exception& e) {
        std::cerr << "ERROR [ " << e.name() << " ] : " << e.message()
            << " at " << e.module() << ":" << e.line() << " in " << e.function() << std::endl;
    }

    //close up any loose ends in log
    logging::close();

    return 0;
}

void printUsage() {
    printf( "Usage: ldmx-app [-v,--verbosity [0-3]] [-f,--fileLog [0-3]] [-h,--help] {config_script.py} [arguments to configuration script]\n" );
    printf( "  -v,--verbosity    Set the verbosity of the logging.\n" );
    printf( "                      0 ==> batch (no logging)\n" );
    printf( "                      1 ==> warnings and errors only (default without -v)\n" );
    printf( "                      2 ==> info, warnings, and errors\n" );
    printf( "                      3 ==> everything (debug, info, warnings and errors; default without following int)\n" );
    printf( "  -f,--fileLog      Print the logs to a file as well as the terminal screen.\n" );
    printf( "                      0 ==> batch (no logging, default without -f)\n" );
    printf( "                      1 ==> warnings and errors only\n" );
    printf( "                      2 ==> info, warnings, and errors\n" );
    printf( "                      3 ==> everything (debug, info, warnings and errors; default without following int)\n" );
    printf( "  -h,--help         Print this help message.\n" );
    printf( "  config_script.py  Configuration script to tell ldmx-app what processors to run.\n" );
    printf( "                    Everything after the config_script is assumed to be arguments to that python script.\n" );
    return;
}
