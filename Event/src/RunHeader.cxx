/**
 * @file RunHeader.cxx
 * @brief Class encapsulating run information such as run #, detector etc.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Event/RunHeader.h"

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>

namespace ldmx {

    RunHeader::RunHeader(int runNumber) : runNumber_(runNumber) { }

    void RunHeader::stream(std::ostream& s) const {
        s << "RunHeader { run: " << getRunNumber()
                << ", detectorName: " << getDetectorName()
                << ", description: " << getDescription()
                << "\n";
        s << "  intParameters: " << "\n";
        for (const auto& [ key , val ] : intParameters_)
            s << "    " << key << " = " << val << "\n";
        s << "  floatParameters: " << "\n";
        for (const auto& [ key , val ] : floatParameters_)
            s << "    " << key << " = " << val << "\n";
        s << "  stringParameters: " << "\n";
        for (const auto& [ key , val ] : stringParameters_)
            s << "    " << key << " = " << val << "\n";
        s << "}";
    }

    void RunHeader::Print() const {
        stream(std::cout);
    }

}
