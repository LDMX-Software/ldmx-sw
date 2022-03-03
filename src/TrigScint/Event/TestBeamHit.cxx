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
    ldmx::TrigScintHit::Clear();
    earlyPedestal_ = -999;
    pedestal_ = -999;
    pulseQ_ = -999;
	startSample_=-1;
	pulseWidth_=-1;
	sampOverThr_=-1; 
  }

  void TestBeamHit::Print(Option_t * option) const {
    std::cout << "TestBeamHit { "
              << "id: " << std::hex << getID() << std::dec
              << ",  energy: " << getEnergy() << "MeV, time: " << getTime()
              << "ns, amplitude: " << getAmplitude() << ", pe: " << getPE()
              << "}" << std::endl;
  }
}  // namespace trigscint
