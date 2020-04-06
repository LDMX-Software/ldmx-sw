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
 * TODO update me
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

    /**
     * Scan for python file
     *
     * While we are looking for it, check for other application arguments
     * Need to know what level to log for the terminal and for the file
     */
    int ptrpy = 1;
    std::vector<std::string> loggingArgs;
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
        } 

        loggingArgs.push_back( currArg );
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
    // this function initializes the backend of the boost logging library
    // using the parameters passed on the command line before the python config file
    logging::open( loggingArgs );

    ldmx::logging::logger lg = logging::makeLogger( "ldmx-app" );

    Process* p { 0 };
    try {
        BOOST_LOG_SEV(lg,level::info) << "----- LDMXSW: Loading configuration file -----";
        
        ConfigurePython cfg(argv[ptrpy], argv + ptrpy + 1, argc - ptrpy);
        p = cfg.makeProcess();

        BOOST_LOG_SEV(lg,level::info) << "----- LDMXSW: Configuration load complete -----";

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

        BOOST_LOG_SEV(lg,level::info) << "----- LDMXSW: Starting event processing -----";
        
        p->run();
        
        BOOST_LOG_SEV(lg,level::info) << "----- LDMXSW: Event processing complete -----";

    } catch (Exception& e) {
        BOOST_LOG_SEV(lg,level::error) << "[ " << e.name() << " ] : " << e.message()
            << " at " << e.module() << ":" << e.line() << " in " << e.function();
    }

    //close up any loose ends in log
    logging::close();

    return 0;
}

void printUsage() {
    printf( "Usage: ldmx-app [-v,--verbosity [0-5]] [-f,--fileLog [0-5] [fileName]] [-h,--help] {config_script.py} [arguments to configuration script]\n" );
    printf( "  -v,--verbosity    Set the verbosity of the logging.\n");
    printf( "  -f,--fileLog      Print the logs to a file as well as the terminal screen.\n" );
    printf( "                      fileName ==> name of file to print log to (must end in '.log', default is 'ldmx_app.log')\n");
    printf( "                    The levels for logging are listed below.\n" );
    printf( "                    When you provide a level, everything equal to it and above will be printed.\n" );
    printf( "                      5 ==> batch (no logging at all; default for file without -f)\n" );
    printf( "                      4 ==> fatals\n" );
    printf( "                      3 ==> errors\n" );
    printf( "                      2 ==> warnings (default for terminal without -v)\n" );
    printf( "                      1 ==> info\n" );
    printf( "                      0 ==> debug (default for file/terminal if given -f/-v without trailing digit)\n");
    printf( "  -h,--help         Print this help message.\n" );
    printf( "  config_script.py  Configuration script to tell ldmx-app what processors to run.\n" );
    printf( "                    Everything after the config_script is assumed to be arguments to that python script.\n" );
    return;
}
