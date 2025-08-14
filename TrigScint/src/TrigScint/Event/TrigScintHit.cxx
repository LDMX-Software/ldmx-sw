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

ClassImp(ldmx::TrigScintHit);

namespace ldmx {
void TrigScintHit::clear(Option_t* option) {
  ldmx::HcalHit::clear();
  bar_id_ = -1;
  module_id_ = -1;
  beam_efrac_ = 0;
}

std::ostream& operator<<(std::ostream& o, const TrigScintHit& c) {
  return o << "TrigScintHit { " << "id: " << std::hex << c.getID() << std::dec
           << ",  energy: " << c.getEnergy() << "MeV, time: " << c.getTime()
           << "ns, amplitude: " << c.getAmplitude() << ", pe: " << c.getPE()
           << "}";
}
}  // namespace ldmx
