/**
 * @file RunHeader.cxx
 * @brief Class encapsulating run information such as run #, detector etc.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Event/RunHeader.h"

namespace ldmx {

    RunHeader::RunHeader(int runNumber, std::string detectorName, 
                 int detectorVersion, std::string description) :
        runNumber_(runNumber), 
        detectorName_(detectorName), 
        detectorVersion_(detectorVersion), 
        description_(description) {
    }


    void RunHeader::Print(Option_t *) const {
        std::cout << "RunHeader { run: " << runNumber_
                << ", detectorName: " << detectorName_
                << ", detectorVersion: " << detectorVersion_
                << ", description: " << description_
                << std::endl;
        std::cout << "  intParameters: " << std::endl;
        for (auto entry : intParameters_) {
            std::cout << "    " << entry.first
                    << " = " << entry.second << std::endl;
        }
        std::cout << "  floatParameters: " << std::endl;
        for (auto entry : floatParameters_) {
            std::cout << "    " << entry.first
                    << " = " << entry.second << std::endl;
        }
        std::cout << "  stringParameters: " << std::endl;
        for (auto entry : stringParameters_) {
            std::cout << "    " << entry.first
                    << " = " << entry.second << std::endl;
        }
        std::cout << "}" << std::endl;
    }

}
