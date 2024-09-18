#ifndef TESTUTILS_H
#define TESTUTILS_H
#include "objdef.h"

bool compareHit(Hit Hit1, Hit Hit2) {
  return ((Hit1.mID == Hit2.mID) and (Hit1.bID == Hit2.bID) and
          (Hit1.Amp == Hit2.Amp) and
          (Hit1.Time == Hit2.Time));  // and(Hit1.TrigTime==Hit2.TrigTime));
}

bool compareClus(Cluster clus1[NHITS], Cluster clus2[NHITS]) {
  for (int i = 0; i < NHITS; ++i) {
    if (not((compareHit(clus1[i].Seed, clus2[i].Seed)) and
            (compareHit(clus1[i].Sec, clus2[i].Sec)))) {
      return false;
    }
  }
  return true;
}

#endif
