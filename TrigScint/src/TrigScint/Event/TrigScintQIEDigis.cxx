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
  return o << "TrigScintQIEDigis { " << "chanID= " << c.chanID_ << ", "
           << "ADC[0]= " << c.adcs_[0] << ", " << "TDC[0]= " << c.tdcs_[0]
           << ", " << "CID[0]= " << c.tdcs_[0] << ", " << "}\n";
}

void TrigScintQIEDigis::Clear(Option_t* option) {}
}  // namespace trigscint
