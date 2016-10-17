#ifndef SimApplication_LHEReader_h
#define SimApplication_LHEReader_h

// LDMX
#include "SimApplication/LHEEvent.h"

// STL
#include <fstream>

class LHEReader {

    public:

        LHEReader(std::string& filename);

        virtual ~LHEReader();

        LHEEvent* readNextEvent();

    private:

        std::ifstream ifs;
};

#endif
