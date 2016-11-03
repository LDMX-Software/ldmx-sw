#ifndef SIMAPPLICATION_LHEREADER_H_
#define SIMAPPLICATION_LHEREADER_H_

// LDMX
#include "SimApplication/LHEEvent.h"

// STL
#include <fstream>

namespace sim {

class LHEReader {

    public:

        LHEReader(std::string& filename);

        virtual ~LHEReader();

        LHEEvent* readNextEvent();

    private:

        std::ifstream ifs_;
};

}

#endif
