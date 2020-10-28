/**
 * @file TrigScintQIEDigis.cxx
 * @brief class for storing QIE output
 * @author Niramay Gogate, Texas Tech University
 */

#include "Event/TrigScintQIEDigis.h"
#include <iostream>
#include <exception>
ClassImp(ldmx::TrigScintQIEDigis);

namespace ldmx {

  TrigScintQIEDigis::TrigScintQIEDigis(int maxTS_=5){
    maxTS = maxTS_;
  }

  void TrigScintQIEDigis::SetADC(int* adc_){
    for(int i=0;i<maxTS;i++){
      ADCs.push_back(adc_[i]);
    }
  }
  
  void TrigScintQIEDigis::SetTDC(int* tdc_){
    for(int i=0;i<maxTS;i++){
      TDCs.push_back(tdc_[i]);
    }
  }
  
  void TrigScintQIEDigis::SetCID(int* cid_){
    for(int i=0;i<maxTS;i++){
      CIDs.push_back(cid_[i]);
    }
  }
  
  void TrigScintQIEDigis::Print(Option_t* option) const {
    std::cout<<"TrigScintQIEDigis { "
	     <<"maxTS= "<<maxTS<<", "
	     <<"chanID= "<<chanID<<", "
	     <<"ADC[0]= "<<ADCs[0]<<", "
	     <<"TDC[0]= "<<TDCs[0]<<", "
	     <<"CID[0]= "<<TDCs[0]<<", "
	     <<"}\n";
  }

  void TrigScintQIEDigis::Clear(Option_t* option){}
}
// DECLARE_PRODUCER_NS(ldmx, TrigScintQIEDigis);
