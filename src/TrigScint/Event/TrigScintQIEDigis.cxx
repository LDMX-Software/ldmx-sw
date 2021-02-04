/**
 * @file TrigScintQIEDigis.cxx
 * @brief class for storing QIE output
 * @author Niramay Gogate, Texas Tech University
 */

#include "TrigScint/Event/TrigScintQIEDigis.h"
#include <iostream>
#include <exception>
ClassImp(ldmx::TrigScintQIEDigis);

namespace ldmx {

  // void TrigScintQIEDigis::SetADC(std::vector<int> adc_) {
  //   adcs = adc_;
  // }
  
  // void TrigScintQIEDigis::SetTDC(std::vector<int> tdc_) {
  //   tdcs = tdc_;
  // }
  
  // void TrigScintQIEDigis::SetCID(std::vector<int> cid_) {
  //   cids = cid_;
  // }
  
  void TrigScintQIEDigis::Print(Option_t* option) const {
    std::cout<<"TrigScintQIEDigis { "
	     <<"chanID= "<<chanID_<<", "
	     <<"ADC[0]= "<<adcs_[0]<<", "
	     <<"TDC[0]= "<<tdcs_[0]<<", "
	     <<"CID[0]= "<<tdcs_[0]<<", "
	     <<"}\n";
  }

  void TrigScintQIEDigis::Clear(Option_t* option) {}
}
