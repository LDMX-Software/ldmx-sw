#ifndef SIMAPPLICATION_SIMAPPLICATION_H_
#define SIMAPPLICATION_SIMAPPLICATION_H_ 1

// STL
#include <string>

class SimApplication {

    public:

        SimApplication();

        virtual ~SimApplication();

        void run(int argc, char** argv);
};

#endif
