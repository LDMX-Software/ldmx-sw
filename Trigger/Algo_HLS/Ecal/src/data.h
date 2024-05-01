#ifndef DATA_H
#define DATA_H

#include "ap_int.h"
#include "ap_fixed.h"


// HGCROC trigger ID and primitive
typedef ap_uint<20> tid_t;
typedef ap_uint<7> tp_t;

typedef ap_uint<18> lin_t;

typedef ap_ufixed<16,14> e_t; // [MeV] (Up to at least 8 GeV)
typedef ap_fixed<10,9> xy_t; // [mm] (-250 to 250, resolution = a few mm)
typedef ap_fixed<16,14> pxy_t; // [MeV/c]


struct EcalTP {
  tid_t tid;
  tp_t tp;
};

struct Ecal2dCluster {
  e_t e;
  xy_t x;
  xy_t y;
};



#endif
