#ifndef HITPRODUCER_H
#define HITPRODUCER_H

#include "objdef.h"

void copyHit1(Hit One, Hit Two);
void copyHit2(Hit One, Hit Two);
void hitproducer_ref(ap_uint<14> FIFO[NHITS][5], Hit outHit[NHITS],
                     ap_uint<8> Peds[NHITS]);
void hitproducer_hw(ap_uint<14> FIFO[NHITS][5], Hit outHit[NHITS],
                    ap_uint<8> Peds[NHITS]);

#endif
