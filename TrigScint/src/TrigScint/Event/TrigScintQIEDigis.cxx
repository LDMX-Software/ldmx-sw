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

void TrigScintQIEDigis::Print(Option_t* option) const {
  std::cout << "TrigScintQIEDigis { "
            << "chanID= " << chanID_ << ", "
            << "ADC[0]= " << adcs_[0] << ", "
            << "TDC[0]= " << tdcs_[0] << ", "
            << "CID[0]= " << tdcs_[0] << ", "
            << "}\n";
}

void TrigScintQIEDigis::Clear(Option_t* option) {}
}  // namespace trigscint
