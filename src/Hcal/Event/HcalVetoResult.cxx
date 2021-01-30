#include "Hcal/Event/HcalVetoResult.h"

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>

//-------------//
//   ldmx-sw   //
//-------------//
#include "Hcal/Event/HcalHit.h"

ClassImp(ldmx::HcalVetoResult)

    namespace ldmx {
  HcalVetoResult::HcalVetoResult() {}

  HcalVetoResult::~HcalVetoResult() {}

  void HcalVetoResult::Clear() { passesVeto_ = false; }

  void HcalVetoResult::Print() const {
    std::cout << "[ HcalVetoResult ]: Passes veto : "
              << " Passes veto: " << passesVeto_ << std::endl;
    maxPEHit_.Print();
  }
}  // namespace ldmx
