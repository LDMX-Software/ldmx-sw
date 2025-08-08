
#include "Framework/RunHeader.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

ClassImp(ldmx::RunHeader);

namespace ldmx {

RunHeader::RunHeader(int run_number) : run_number_(run_number) {}

void RunHeader::stream(std::ostream &s) const {
  s << "RunHeader { run: " << getRunNumber() << ", numTries: " << getNumTries()
    << ", detectorName: " << getDetectorName()
    << ", description: " << getDescription() << "\n";
  s << "  intParameters: " << "\n";
  for (const auto &[key, val] : int_parameters_)
    s << "    " << key << " = " << val << "\n";
  s << "  floatParameters: " << "\n";
  for (const auto &[key, val] : float_parameters_)
    s << "    " << key << " = " << val << "\n";
  s << "  stringParameters: " << "\n";
  for (const auto &[key, val] : string_parameters_)
    s << "    " << key << " = " << val << "\n";
  s << "}";
}

void RunHeader::print() const { stream(std::cout); }

}  // namespace ldmx
