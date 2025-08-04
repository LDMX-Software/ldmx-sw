#include "Hcal/Event/HcalVetoResult.h"

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>

//-------------//
//   ldmx-sw   //
//-------------//
#include "Hcal/Event/HcalHit.h"

ClassImp(ldmx::HcalVetoResult);

namespace ldmx {

void HcalVetoResult::Clear() { passes_veto_ = false; }

std::ostream& operator<<(std::ostream& o, const HcalVetoResult& c) {
  return o << "[ HcalVetoResult ]: Passes veto : " << " Passes veto: "
           << c.passes_veto_ << "with max hitL " << c.max_PE_hit_;
}
}  // namespace ldmx
