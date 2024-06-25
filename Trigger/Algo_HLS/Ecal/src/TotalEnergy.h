#ifndef TOTALENERGY_H
#define TOTALENERGY_H

#include <cmath>
#include <iostream>

#include "ap_fixed.h"
#include "ap_int.h"
#include "data.h"

#define N_INPUT_TP 100
#define N_CLUSTER 10

void TotalEnergy_hw(EcalTP Input_TPs[N_INPUT_TP], e_t &energy);
void TotalEnergy_ref(EcalTP Input_TPs[N_INPUT_TP], e_t &energy);

#endif
