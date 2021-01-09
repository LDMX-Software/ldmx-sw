#include "Hcal/Event/HcalVetoResult.h"

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>

//-------------//
//   ldmx-sw   //
//-------------//
#include "Hcal/Event/HcalHit.h"

ClassImp(hcal::event::HcalVetoResult)

namespace hcal {
namespace event {
HcalVetoResult::HcalVetoResult() {}

HcalVetoResult::~HcalVetoResult() {}

void HcalVetoResult::Clear() { passesVeto_ = false; }

void HcalVetoResult::Print() const {
  std::cout << "[ HcalVetoResult ]: Passes veto : "
            << " Passes veto: " << passesVeto_ << std::endl;
  maxPEHit_.Print();
}
} // namespace event
} // namespace hcal
