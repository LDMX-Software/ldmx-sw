#include "DetDescr/HcalElectronicsID.h"
#include "DetDescr/DetectorIDInterpreter.h"

std::ostream& operator<<(std::ostream& s, const ldmx::HcalElectronicsID& id) {
  s << "HcalElectronics(" << id.polarfire() << ',' << id.roc() << ',' << id.channel() << ')';
  return s;
}
