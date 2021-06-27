#include "DetDescr/EcalElectronicsID.h"
#include "DetDescr/DetectorIDInterpreter.h"

std::ostream& operator<<(std::ostream& s, const ldmx::EcalElectronicsID& id) {
  s << "EcalElectronics(" << id.fiber() << ',' << id.elink() << ',' << id.channel() << ')';
  return s;
}
