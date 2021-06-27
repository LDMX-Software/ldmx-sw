#include "DetDescr/HcalAbstractID.h"
#include "DetDescr/HcalID.h"
#include "DetDescr/HcalDigiID.h"
#include <iomanip>

std::ostream& operator<<(std::ostream& s, const ldmx::HcalAbstractID& id) {
  switch (id.bar_type()) {
    case(ldmx::HcalAbstractID::Global): return s << ldmx::HcalID(id);
    case(ldmx::HcalAbstractID::Digi): return s << ldmx::HcalDigiID(id);
      //    case(ldmx::HcalAbstractID::Trigger): return s << ldmx::HcalTriggerID(id);
    default:
      return s << "HcalAbstractID("<<id.bar_type()<<":0x"<<std::setfill('0') << std::setw(6) << std::right << std::hex << id.payload();
  }
    
}
