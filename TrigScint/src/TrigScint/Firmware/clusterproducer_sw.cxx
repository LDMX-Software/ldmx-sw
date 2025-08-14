#include <stdio.h>

#include <array>
#include <iostream>

#include "TrigScint/Firmware/clusterproducer.h"
#include "TrigScint/Firmware/objdef.h"

std::array<Cluster, NCLUS> clusterproducerSw(Hit inHit[NHITS]) {
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
    if (inHit[j].b_id_ > -1) {
      map_l1[inHit[j].b_id_] = j;
    }
  }
  // NOW WE JUST LOOK FOR HITS EXCEEDING SEED, IF THEY DO WE PAIR 'EM.
  for (int k = 0; k < NCLUS; ++k) {
    bool do_next_cluster = true;
    if ((map_l1[2 * k] > -1)) {
      if (inHit[map_l1[2 * k]].amp_ > seedthr) {
        clearClus(out_clus[k]);
        out_clus[k].seed_.m_id_ = inHit[map_l1[2 * k]].m_id_;
        out_clus[k].seed_.b_id_ = inHit[map_l1[2 * k]].b_id_;
        out_clus[k].seed_.amp_ = inHit[map_l1[2 * k]].amp_;
        out_clus[k].seed_.time_ = inHit[map_l1[2 * k]].time_;
        if (map_l1[2 * k + 1] > -1) {
          if (inHit[map_l1[2 * k + 1]].amp_ > clusthr) {
            out_clus[k].sec_.m_id_ = inHit[map_l1[2 * k + 1]].m_id_;
            out_clus[k].sec_.b_id_ = inHit[map_l1[2 * k + 1]].b_id_;
            out_clus[k].sec_.amp_ = inHit[map_l1[2 * k + 1]].amp_;
            out_clus[k].sec_.time_ = inHit[map_l1[2 * k + 1]].time_;
            do_next_cluster = false;
            // You can comment this line to turn it into Serialized
            clearHit(inHit[map_l1[2 * k + 1]]);
          }
        }
      }
    }
    if ((map_l1[2 * k + 1] > -1) and (do_next_cluster)) {
      if (inHit[map_l1[2 * k + 1]].amp_ > seedthr) {
        clearClus(out_clus[k]);
        out_clus[k].seed_.m_id_ = inHit[map_l1[2 * k + 1]].m_id_;
        out_clus[k].seed_.b_id_ = inHit[map_l1[2 * k + 1]].b_id_;
        out_clus[k].seed_.amp_ = inHit[map_l1[2 * k + 1]].amp_;
        out_clus[k].seed_.time_ = inHit[map_l1[2 * k + 1]].time_;
        if (k < NCLUS - 1) {
          if (map_l1[2 * k + 2] > -1) {
            if (inHit[map_l1[2 * k + 2]].amp_ > clusthr) {
              out_clus[k].sec_.m_id_ = inHit[map_l1[2 * k + 2]].m_id_;
              out_clus[k].sec_.b_id_ = inHit[map_l1[2 * k + 2]].b_id_;
              out_clus[k].sec_.amp_ = inHit[map_l1[2 * k + 2]].amp_;
              out_clus[k].sec_.time_ = inHit[map_l1[2 * k + 2]].time_;
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
