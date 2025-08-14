#include <stdio.h>

#include <iostream>

#include "TrigScint/Firmware/objdef.h"
#include "TrigScint/Firmware/trackproducer.h"

void trackproducerHw(Cluster pad1[NTRK], Cluster pad2[NCLUS],
                     Cluster pad3[NCLUS], Track outTrk[NTRK],
                     ap_int<12> lookup[NCENT][COMBO][2]) {
#ifdef TS_NOT_EMULATION
#pragma HLS ARRAY_PARTITION variable = pad1 dim = 0 complete
#pragma HLS ARRAY_PARTITION variable = pad2 dim = 0 complete
#pragma HLS ARRAY_PARTITION variable = pad3 dim = 0 complete
#pragma HLS ARRAY_PARTITION variable = outTrk dim = 0 complete
#pragma HLS ARRAY_PARTITION variable = lookup dim = 0 complete
#pragma HLS PIPELINE II = 10
#endif
  Track test;
#ifdef TS_NOT_EMULATION
#pragma HLS ARRAY_PARTITION variable = test complete
#endif

  // This firmware module loops over first the pad1 seeds (NTRK) and then the
  // patterns (COMBO) For each seed it check 9 combinations of tracks. These
  // combinations, which depend on alignment essentially consist of the clusters
  // that have channels immediattely above or below the pad1 cluster in the
  // first layer, which you may observe from the LUT if you printed it. I would
  // only need to check the pattern without all these continue statements, but
  // the continue statements further reduce the pattern collection size by only
  // applying certain patterns iff a secondary hit is there Thats why this looks
  // complicated at all: the continues just include logic on whether a pattern
  // should have a secondary hit. It also checks the track residual, only
  // keeping one pattern for each pad1 cluster.
  for (int i = 0; i < NTRK; i++) {
    if (2 * pad1[i].seed_.b_id_ > 2 * NCHAN) {
      continue;
    }
    for (int j = 0; j < COMBO; j++) {
      clearTrack(test);
      if (not(pad1[i].seed_.amp_ > 0)) {
        continue;
      }  // Continue if.seed_.not Satisfied
      ap_int<12> centroid = 2 * pad1[i].seed_.b_id_;
      if (pad1[i].sec_.amp_ > 0) {
        centroid += 1;
      }
      cpyCluster(test.pad1_, pad1[i]);
      if ((lookup[centroid][j][0] == -1) or (lookup[centroid][j][1] == -1)) {
        continue;
      }  // Pattern Empty
      if (not(pad2[lookup[centroid][j][0] / 4].seed_.amp_ > 0)) {
        continue;
      }  // Continue if.seed_.not Satisfied
      if ((lookup[centroid][j][0] % 4 == 0) and
          ((pad2[lookup[centroid][j][0] / 4].sec_.b_id_ >= 0) or
           (pad2[lookup[centroid][j][0] / 4].seed_.b_id_ % 2 == 1))) {
        continue;
      }  // Continue if sec_ is not Expected, and not Empty
      if ((lookup[centroid][j][0] % 4 == 1) and
          ((pad2[lookup[centroid][j][0] / 4].sec_.b_id_ < 0) or
           (pad2[lookup[centroid][j][0] / 4].seed_.b_id_ % 2 == 1))) {
        continue;
      }  // Continue if sec_ is Expected, and Empty
      if ((lookup[centroid][j][0] % 4 == 2) and
          ((pad2[lookup[centroid][j][0] / 4].sec_.b_id_ >= 0) or
           (pad2[lookup[centroid][j][0] / 4].seed_.b_id_ % 2 == 0))) {
        continue;
      }  // Continue if sec_ is not Expected, and not Empty
      if ((lookup[centroid][j][0] % 4 == 3) and
          ((pad2[lookup[centroid][j][0] / 4].sec_.b_id_ < 0) or
           (pad2[lookup[centroid][j][0] / 4].seed_.b_id_ % 2 == 0))) {
        continue;
      }  // Continue if sec_ is Expected, and Empty
      if (not(pad3[lookup[centroid][j][1] / 4].seed_.amp_ > 0)) {
        continue;
      }  // Continue if.seed_.not Satisfied
      if ((lookup[centroid][j][1] % 4 == 0) and
          ((pad3[lookup[centroid][j][1] / 4].sec_.b_id_ >= 0) or
           (pad3[lookup[centroid][j][1] / 4].seed_.b_id_ % 2 == 1))) {
        continue;
      }  // Continue if sec_ is not Expected, and not Empty
      if ((lookup[centroid][j][1] % 4 == 1) and
          ((pad3[lookup[centroid][j][1] / 4].sec_.b_id_ < 0) or
           (pad3[lookup[centroid][j][1] / 4].seed_.b_id_ % 2 == 1))) {
        continue;
      }  // Continue if sec_ is Expected, and Empty
      if ((lookup[centroid][j][1] % 4 == 2) and
          ((pad3[lookup[centroid][j][1] / 4].sec_.b_id_ >= 0) or
           (pad3[lookup[centroid][j][1] / 4].seed_.b_id_ % 2 == 0))) {
        continue;
      }  // Continue if sec_ is not Expected, and not Empty
      if ((lookup[centroid][j][1] % 4 == 3) and
          ((pad3[lookup[centroid][j][1] / 4].sec_.b_id_ < 0) or
           (pad3[lookup[centroid][j][1] / 4].seed_.b_id_ % 2 == 0))) {
        continue;
      }  // Continue if sec_ is Expected, and Empty
      cpyCluster(test.pad2_, pad2[lookup[centroid][j][0] / 4]);
      cpyCluster(test.pad3_, pad3[lookup[centroid][j][1] / 4]);
      calcResid(test);
      if (test.resid_ < outTrk[i].resid_) {
        cpyTrack(outTrk[i], test);
      }
    }  // end loop on COMBO
  }  // end loop on NTRK
  // While we ultimately envision having the firmware do duplicate track removal
  // in the other two layers in a separate firmware module, they are done here
  // so as to not have track over counting and to validate the processor. Thats
  // what occurs here below.
  for (int i = 1; i < NTRK - 1; i++) {
    if ((outTrk[i - 1].pad2_.seed_.b_id_ == outTrk[i].pad2_.seed_.b_id_) and
        (outTrk[i].pad2_.seed_.b_id_ >= 0)) {
      if (outTrk[i - 1].resid_ <= outTrk[i].resid_) {
        clearTrack(outTrk[i]);
      } else {
        clearTrack(outTrk[i - 1]);
      }
    }
    if ((outTrk[i].pad2_.seed_.b_id_ == outTrk[i + 1].pad2_.seed_.b_id_) and
        (outTrk[i + 1].pad2_.seed_.b_id_ >= 0)) {
      if (outTrk[i + 1].resid_ <= outTrk[i].resid_) {
        clearTrack(outTrk[i]);
      } else {
        clearTrack(outTrk[i + 1]);
      }
    }
    if ((outTrk[i - 1].pad2_.seed_.b_id_ == outTrk[i + 1].pad2_.seed_.b_id_) and
        (outTrk[i + 1].pad2_.seed_.b_id_ >= 0)) {
      if (outTrk[i - 1].resid_ <= outTrk[i + 1].resid_) {
        clearTrack(outTrk[i + 1]);
      } else {
        clearTrack(outTrk[i - 1]);
      }
    }
  }  // end loop on NTRK
  for (int i = 1; i < NTRK - 1; i++) {
    if ((outTrk[i - 1].pad3_.seed_.b_id_ == outTrk[i].pad3_.seed_.b_id_) and
        (outTrk[i].pad3_.seed_.b_id_ >= 0)) {
      if (outTrk[i - 1].resid_ <= outTrk[i].resid_) {
        clearTrack(outTrk[i]);
      } else {
        clearTrack(outTrk[i - 1]);
      }
    }
    if ((outTrk[i].pad3_.seed_.b_id_ == outTrk[i + 1].pad3_.seed_.b_id_) and
        (outTrk[i + 1].pad3_.seed_.b_id_ >= 0)) {
      if (outTrk[i + 1].resid_ <= outTrk[i].resid_) {
        clearTrack(outTrk[i]);
      } else {
        clearTrack(outTrk[i + 1]);
      }
    }
    if ((outTrk[i - 1].pad3_.seed_.b_id_ == outTrk[i + 1].pad3_.seed_.b_id_) and
        (outTrk[i + 1].pad3_.seed_.b_id_ >= 0)) {
      if (outTrk[i - 1].resid_ <= outTrk[i + 1].resid_) {
        clearTrack(outTrk[i + 1]);
      } else {
        clearTrack(outTrk[i - 1]);
      }
    }
  }  // end loop on NTRK again
  return;
}
