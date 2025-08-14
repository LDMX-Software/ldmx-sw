#ifndef TESTUTILS_H
#define TESTUTILS_H
#include "objdef.h"

bool compareHit(Hit Hit1, Hit Hit2) {
  return ((Hit1.m_id_ == Hit2.m_id_) and (Hit1.b_id_ == Hit2.b_id_) and
          (Hit1.amp_ == Hit2.amp_) and
          (Hit1.time_ == Hit2.time_));  // and(Hit1.TrigTime==Hit2.TrigTime));
}

bool compareClus(Cluster clus1[NHITS], Cluster clus2[NHITS]) {
  for (int i = 0; i < NHITS; ++i) {
    if (not((compareHit(clus1[i].seed_, clus2[i].seed_)) and
            (compareHit(clus1[i].sec_, clus2[i].sec_)))) {
      return false;
    }
  }
  return true;
}

#endif
