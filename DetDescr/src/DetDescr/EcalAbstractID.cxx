#include "DetDescr/EcalAbstractID.h"
#include "DetDescr/EcalID.h"
#include "DetDescr/EcalTriggerID.h"
#include <iomanip>

std::ostream& operator<<(std::ostream& s, const ldmx::EcalAbstractID& id) {
  switch (id.cell_type()) {
    case(ldmx::EcalAbstractID::PrecisionGlobal): return s << ldmx::EcalID(id);
    case(ldmx::EcalAbstractID::TriggerCell): return s << ldmx::EcalTriggerID(id);
    default:
      return s << "EcalAbstractID("<<id.cell_type()<<":0x"<<std::setfill('0') << std::setw(6) << std::right << std::hex << id.payload();
  }
    
}
