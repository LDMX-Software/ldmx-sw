/**
 * @file TestBeamHit.cxx
 * @brief Class that stores reconstructed hit information from the TS test
 * stand/test beam
 * @author Lene Kristian Bryngemark, Stanford University
 */

#include "TrigScint/Event/TestBeamHit.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

ClassImp(trigscint::TestBeamHit);

namespace trigscint {
void TestBeamHit::clear(Option_t* option) {
  early_pedestal_ = -999;
  pedestal_ = -999;
  pulse_q_ = -999;
  start_sample_ = -1;
  pulse_width_ = -1;
  samp_above_thr_ = -1;
}

std::ostream& operator<<(std::ostream& o, const TestBeamHit& c) {
  return o << "TestBeamHit { " << "Total charge: " << c.getQ()
           << " fC, start time sample: " << c.getStartSample()
           << ", bar: " << c.getBarID() << ", pulseWidth: " << c.getPulseWidth()
           << "}";
}
}  // namespace trigscint
