#include "Event/RunHeader.h"

// STL
#include <iostream>

namespace ldmx {

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
