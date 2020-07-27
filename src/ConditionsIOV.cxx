#include "Framework/ConditionsIOV.h"
#include <iostream>
#include <sstream>

namespace ldmx {
    void ConditionsIOV::Print() const {
	std::cout << ToString() << std::endl;
    }

    std::string ConditionsIOV::ToString() const {
	std::stringstream s;
	s << "IOV(" << firstRun_ << "->";
	if (lastRun_==-1) s << "[all runs]";
	else s << lastRun_;
	if (validForData_) s << ",Data";
	if (validForMC_) s << ",MC";
	s << ")";

	return s.str();
    }

}
