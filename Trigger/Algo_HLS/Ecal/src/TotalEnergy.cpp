/*
  HLS implementation of the missing energy calculation
*/
#include "TotalEnergy.h"

void TotalEnergy_hw(EcalTP Input_TPs[N_INPUT_TP], e_t &energy){

  energy=0;
  for(int i=0;i<N_INPUT_TP;i++){
    //TODO add conversion to linear
    energy += Input_TPs[i].tp;
  }
}
