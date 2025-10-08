#include "DetDescr/HcalElectronicsID.h"

#include "DetDescr/DetectorIDInterpreter.h"

namespace ldmx {
std::ostream& operator<<(std::ostream& s, const ldmx::HcalElectronicsID& id) {
  s << "HcalElectronics(" << id.fiber() << ',' << id.elink() << ','
    << id.channel() << ')';
  return s;
}
}  // namespace ldmx