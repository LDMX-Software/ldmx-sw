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

void HcalVetoResult::Print() const {
  std::cout << "[ HcalVetoResult ]: Passes veto : " << " Passes veto: "
            << passes_veto_ << std::endl;
  max_PE_hit_.Print();
}
}  // namespace ldmx
