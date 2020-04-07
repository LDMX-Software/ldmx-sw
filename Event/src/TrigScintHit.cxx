/**
 * @file Trigscinthit.cxx
 * @brief Class that stores Stores reconstructed hit information from the HCAL
 * @author Andrew Whitbeck, Texas Tech University
 */

#include "Event/TrigScintHit.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

ClassImp(ldmx::TrigScintHit)

namespace ldmx {

    void TrigScintHit::Clear(Option_t *option) {
        HcalHit::Clear();
        setPE(0);
    }

    void TrigScintHit::Print(std::ostream& o) const {
        o << "TrigScintHit { " << "id: " << std::hex << getID() << std::dec
          << ",  energy: " << getEnergy() << "MeV, time: " << getTime()
          << "ns, amplitude: " << getAmplitude() << ", pe: " << getPE() << "}";
    }

}
