#include "SimCore/UserEventInformation.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

namespace simcore {

void UserEventInformation::Print() const {
  std::cout << "Event weight: " << weight_ << "\n"
            << "Brem candidate count: " << bremCandidateCount_ << "\n"
            << "E_{PN} = " << total_photonuclear_energy_ << " MeV  "
            << "E_{EN} = " << total_electronuclear_energy_ << " MeV"
            << std::endl;
}
}  // namespace simcore
