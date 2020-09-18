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

std::ostream& operator<<(std::ostream& s, const ldmx::RunHeader& h) {
    s << "RunHeader { run: " << h.getRunNumber()
            << ", detectorName: " << h.getDetectorName()
            << ", description: " << h.getDescription()
            << "\n";
    s << "  intParameters: " << "\n";
    for (const auto& [ key , val ] : h.getIntParameters())
        s << "    " << key << " = " << val << "\n";
    s << "  floatParameters: " << "\n";
    for (const auto& [ key , val ] : h.getFloatParameters())
        s << "    " << key << " = " << val << "\n";
    s << "  stringParameters: " << "\n";
    for (const auto& [ key , val ] : h.getStringParameters())
        s << "    " << key << " = " << val << "\n";
    s << "}";
}

namespace ldmx {

    RunHeader::RunHeader(int runNumber) : runNumber_(runNumber) { }

    void RunHeader::Print() const {
        std::cout << *this << std::endl;
    }

}
