/**
 * @file TestBeamHit.cxx
 * @brief Class that stores reconstructed hit information from the TS test stand/test beam 
 * @author Lene Kristian Bryngemark, Stanford University
 */

#include "TrigScint/Event/TestBeamHit.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

ClassImp(trigscint::TestBeamHit)

    namespace trigscint {
  void TestBeamHit::Clear(Option_t * option) {
    earlyPedestal_ = -999;
    pedestal_ = -999;
    pulseQ_ = -999;
	startSample_=-1;
	pulseWidth_=-1;
	sampAboveThr_=-1; 
  }

  void TestBeamHit::Print(Option_t * option) const {
    std::cout << "TestBeamHit { "
              << "Total charge: " << getQ() << " fC, start time sample: " << getStartSample()
              << ", bar: " << getBarID() 
              << ", pulseWidth: " << getPulseWidth() 
              << "}" << std::endl;
  }
}  // namespace trigscint
