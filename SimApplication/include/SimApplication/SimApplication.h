#ifndef SimApplication_SimApplication_h
#define SimApplication_SimApplication_h

// STL
#include <string>

class SimApplication {

    public:

        SimApplication();

        virtual ~SimApplication();

        void run(int argc, char** argv);
};

#endif
