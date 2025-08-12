#include <stdio.h>

#include <array>
#include <iostream>

#include "TrigScint/Firmware/clusterproducer.h"
#include "TrigScint/Firmware/objdef.h"

std::array<Cluster, NCLUS> clusterproducer_sw(Hit inHit[NHITS]) {
  ap_int<12> seedthr = 30;
  ap_int<12> clusthr = 30;

  ap_int<12> map_l1[NCHAN];

  std::array<Cluster, NCLUS> out_clus;

  for (int i = 0; i < NCLUS; ++i) {
    clearClus(out_clus[i]);
  }

  // CLEAR THE MAP
  for (int i = 0; i < NCHAN; ++i) {
    map_l1[i] = -1;
  }
  // MAP TO CHANNELS
  for (int j = 0; j < NHITS; ++j) {
    if (inHit[j].bID > -1) {
      map_l1[inHit[j].bID] = j;
    }
  }
  // NOW WE JUST LOOK FOR HITS EXCEEDING SEED, IF THEY DO WE PAIR 'EM.
  for (int k = 0; k < NCLUS; ++k) {
    bool do_next_cluster = true;
    if ((map_l1[2 * k] > -1)) {
      if (inHit[map_l1[2 * k]].Amp > seedthr) {
        clearClus(out_clus[k]);
        out_clus[k].Seed.mID = inHit[map_l1[2 * k]].mID;
        out_clus[k].Seed.bID = inHit[map_l1[2 * k]].bID;
        out_clus[k].Seed.Amp = inHit[map_l1[2 * k]].Amp;
        out_clus[k].Seed.Time = inHit[map_l1[2 * k]].Time;
        if (map_l1[2 * k + 1] > -1) {
          if (inHit[map_l1[2 * k + 1]].Amp > clusthr) {
            out_clus[k].Sec.mID = inHit[map_l1[2 * k + 1]].mID;
            out_clus[k].Sec.bID = inHit[map_l1[2 * k + 1]].bID;
            out_clus[k].Sec.Amp = inHit[map_l1[2 * k + 1]].Amp;
            out_clus[k].Sec.Time = inHit[map_l1[2 * k + 1]].Time;
            do_next_cluster = false;
            // You can comment this line to turn it into Serialized
            clearHit(inHit[map_l1[2 * k + 1]]);
          }
        }
      }
    }
    if ((map_l1[2 * k + 1] > -1) and (do_next_cluster)) {
      if (inHit[map_l1[2 * k + 1]].Amp > seedthr) {
        clearClus(out_clus[k]);
        out_clus[k].Seed.mID = inHit[map_l1[2 * k + 1]].mID;
        out_clus[k].Seed.bID = inHit[map_l1[2 * k + 1]].bID;
        out_clus[k].Seed.Amp = inHit[map_l1[2 * k + 1]].Amp;
        out_clus[k].Seed.Time = inHit[map_l1[2 * k + 1]].Time;
        if (k < NCLUS - 1) {
          if (map_l1[2 * k + 2] > -1) {
            if (inHit[map_l1[2 * k + 2]].Amp > clusthr) {
              out_clus[k].Sec.mID = inHit[map_l1[2 * k + 2]].mID;
              out_clus[k].Sec.bID = inHit[map_l1[2 * k + 2]].bID;
              out_clus[k].Sec.Amp = inHit[map_l1[2 * k + 2]].Amp;
              out_clus[k].Sec.Time = inHit[map_l1[2 * k + 2]].Time;
              // You can comment this line to turn it into Serialized
              clearHit(inHit[map_l1[2 * k + 2]]);
            }
          }
        }
      }
    }
  }

  return out_clus;
}
