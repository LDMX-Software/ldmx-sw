/**
 * @file Trigscinthit.cxx
 * @brief Class that stores Stores reconstructed hit information from the HCAL
 * @author Andrew Whitbeck, Texas Tech University
 */

#include "TrigScint/Event/TrigScintHit.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

ClassImp(ldmx::TrigScintHit)

    namespace ldmx {
  void TrigScintHit::Clear(Option_t * option) {
    ldmx::HcalHit::Clear();
    barID_ = -1;
    moduleID_ = -1;
    beamEfrac_ = 0;
  }

  void TrigScintHit::Print(Option_t * option) const {
    std::cout << "TrigScintHit { "
              << "id: " << std::hex << getID() << std::dec
              << ",  energy: " << getEnergy() << "MeV, time: " << getTime()
              << "ns, amplitude: " << getAmplitude() << ", pe: " << getPE()
              << "}" << std::endl;
  }
}  // namespace ldmx
