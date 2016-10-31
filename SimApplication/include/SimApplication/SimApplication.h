#ifndef SIMAPPLICATION_SIMAPPLICATION_H_
#define SIMAPPLICATION_SIMAPPLICATION_H_

// STL
#include <string>

namespace sim {

class SimApplication {

    public:

        SimApplication();

        virtual ~SimApplication();

        void run(int argc, char** argv);
};

}

#endif
