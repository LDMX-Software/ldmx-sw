/**
 * @file RunHeader.cxx
 * @brief Class encapsulating run information such as run #, detector etc.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Event/RunHeader.h"

namespace ldmx {

    RunHeader::RunHeader(int runNumber, std::string detectorName, 
                         std::string description) :
        runNumber_(runNumber), 
        detectorName_(detectorName), 
        description_(description) {
    }

    void RunHeader::Print(std::ostream& o) const {
        o << "RunHeader { run: " << runNumber_
          << ", detectorName: " << detectorName_
          << ", description: " << description_
          << ", intParameters: [";
        for (auto it = intParameters_.begin(); it != intParameters_.end(); ++it ) {
            o << it->first << " = " << it->second;
            if (std::next(it,1) == intParameters_.end()) o << "]";
            else o << ", ";
        }
        o << ", floatParameters: [";
        for (auto it = floatParameters_.begin(); it != floatParameters_.end(); ++it ) {
            o << it->first << " = " << it->second;
            if (std::next(it,1) == floatParameters_.end()) o << "]";
            else o << ", ";
        }
        o << ", stringParameters: [";
        for (auto it = stringParameters_.begin(); it != stringParameters_.end(); ++it ) {
            o << it->first << " = " << it->second;
            if (std::next(it,1) == stringParameters_.end()) o << "]";
            else o << ", ";
        }
        o << "}";
    }

}
