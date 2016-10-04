#ifndef SIMAPPLICATION_LHEREADER_H_
#define SIMAPPLICATION_LHEREADER_H_ 1

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
