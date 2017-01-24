/**
 * @file LHEReader.h
 * @brief Class for reading LHE event data
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_LHEREADER_H_
#define SIMAPPLICATION_LHEREADER_H_

// LDMX
#include "SimApplication/LHEEvent.h"

// STL
#include <fstream>

namespace ldmx {

/**
 * @class LHEReader
 * @brief Reads LHE event data into an LHEEvent object
 */
class LHEReader {

    public:

        /**
         * Class constructor.
         * @param fileName The input file name.
         */
        LHEReader(std::string& fileName);

        /**
         * Class destructor.
         */
        virtual ~LHEReader();

        /**
         * Read the next event.
         * @return The next LHE event.
         */
        LHEEvent* readNextEvent();

    private:

        /**
         * The input file stream.
         */
        std::ifstream ifs_;
};

}

#endif
