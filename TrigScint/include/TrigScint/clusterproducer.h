#ifndef CLUSTERPRODUCER_H
#define CLUSTERPRODUCER_H

#include "objdef.h"

void copyHit1(Hit One, Hit Two);
void copyHit2(Hit One, Hit Two);
void clusterproducer_ref(Hit inHit[NHITS],Cluster outClus[NCLUS]);
std::unique_ptr<Cluster []> clusterproducer_sw(Hit inHit[NHITS]);
void clusterproducer_hw(Hit inHit[NHITS],Cluster outClus[NCLUS]);

#endif
