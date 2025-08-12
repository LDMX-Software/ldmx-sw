#ifndef TRACKPRODUCER_H
#define TRACKPRODUCER_H

#include "objdef.h"

void copyCluster1(Cluster One, Cluster Two);
void copyCluster2(Cluster One, Cluster Two);
void trackproducerRef(Cluster Pad1[NTRK], Cluster Pad2[NCLUS],
                       Cluster Pad3[NCLUS], Track outTrk[NTRK],
                       ap_int<12> lookup[NCENT][COMBO][2]);
void trackproducerHw(Cluster Pad1[NTRK], Cluster Pad2[NCLUS],
                      Cluster Pad3[NCLUS], Track outTrk[NTRK],
                      ap_int<12> lookup[NCENT][COMBO][2]);

#endif
