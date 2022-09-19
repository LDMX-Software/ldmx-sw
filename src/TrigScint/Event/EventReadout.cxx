/**
 * @file EventReadout.cxx
 * @brief class for storing linearized QIE output
 * @author Lene Kristian Bryngemark, Stanford University
 */

#include "TrigScint/Event/EventReadout.h"
#include <exception>
#include <iostream>
ClassImp(trigscint::EventReadout);

namespace trigscint {

void EventReadout::Print(Option_t* option) const {
  std::cout << "EventReadout { "
            << "chanID= " << chanID_ << ", "
            << "ADC[0]= " << adcs_[0] << ", "
            << "charge[0]= " << qs_[0] << ", "
            << "pedestal= " << pedestal_ << ", "
            << "noise= " << noise_ << ", "
            << "}\n";
}

void EventReadout::Clear(Option_t* option) {}
}  // namespace trigscint
