/**
 * @file SimApplication.h
 * @brief Class providing the entry point to run the simulation application
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_SIMAPPLICATION_H_
#define SIMAPPLICATION_SIMAPPLICATION_H_

// STL
#include <string>

/**
 * @namespace sim
 * @brief Simulation application classes
 */
namespace sim {

/**
 * @class SimApplication
 * @brief Entry point for initializing and running the <i>ldmx-sim</i> application
 */
class SimApplication {

    public:

        /**
         * Class constructor.
         */
        SimApplication();

        /**
         * Class destructor.
         */
        virtual ~SimApplication();

        /**
         * Run the application with arguments passed from <i>main()</i>.
         * @param argc The command line argument count.
         * @param argv The command line arguments.
         */
        void run(int argc, char** argv);
};

}

#endif
