/**
 * @file TrigScintQIEDigis.cxx
 * @brief class for storing QIE output
 * @author Niramay Gogate, Texas Tech University
 */

#include "TrigScint/Event/TrigScintQIEDigis.h"

#include <exception>
#include <iostream>
ClassImp(trigscint::TrigScintQIEDigis);

namespace trigscint {

std::ostream& operator<<(std::ostream& o, const TrigScintQIEDigis& c) {
  return o << "TrigScintQIEDigis { " << "chan_id = " << c.chan_id_ << ", "
           << "ADC[0]= " << c.adcs_[0] << ", " << "TDC[0]= " << c.tdcs_[0]
           << ", " << "CID[0]= " << c.tdcs_[0] << ", " << "}\n";
}

void TrigScintQIEDigis::clear(Option_t* option) {}
}  // namespace trigscint
