#include <stdio.h>

#include <iostream>

#include "TrigScint/Firmware/objdef.h"
#include "TrigScint/Firmware/trackproducer.h"

void trackproducer_hw(Cluster Pad1[NTRK], Cluster Pad2[NCLUS],
                      Cluster Pad3[NCLUS], Track outTrk[NTRK],
                      ap_int<12> lookup[NCENT][COMBO][2]) {
#pragma HLS ARRAY_PARTITION variable = Pad1 dim = 0 complete
#pragma HLS ARRAY_PARTITION variable = Pad2 dim = 0 complete
#pragma HLS ARRAY_PARTITION variable = Pad3 dim = 0 complete
#pragma HLS ARRAY_PARTITION variable = outTrk dim = 0 complete
#pragma HLS ARRAY_PARTITION variable = lookup dim = 0 complete
#pragma HLS PIPELINE II = 10
  Track test;
#pragma HLS ARRAY_PARTITION variable = test complete

  // This firmware module loops over first the Pad1 seeds (NTRK) and then the
  // patterns (COMBO) For each seed it check 9 combinations of tracks. These
  // combinations, which depend on alignment essentially consist of the clusters
  // that have channels immediattely above or below the Pad1 cluster in the
  // first layer, which you may observe from the LUT if you printed it. I would
  // only need to check the pattern without all these continue statements, but
  // the continue statements further reduce the pattern collection size by only
  // applying certain patterns iff a secondary hit is there Thats why this looks
  // complicated at all: the continues just include logic on whether a pattern
  // should have a secondary hit. It also checks the track residual, only
  // keeping one pattern for each pad1 cluster.
  for (int i = 0; i < NTRK; i++) {
    if (2 * Pad1[i].Seed.bID > 2 * NCHAN) {
      continue;
    }
    for (int I = 0; I < COMBO; I++) {
      clearTrack(test);
      if (not(Pad1[i].Seed.Amp > 0)) {
        continue;
      }  // Continue if Seed not Satisfied
      ap_int<12> centroid = 2 * Pad1[i].Seed.bID;
      if (Pad1[i].Sec.Amp > 0) {
        centroid += 1;
      }
      cpyCluster(test.Pad1, Pad1[i]);
      if ((lookup[centroid][I][0] == -1) or (lookup[centroid][I][1] == -1)) {
        continue;
      }  // Pattern Empty
      if (not(Pad2[lookup[centroid][I][0] / 4].Seed.Amp > 0)) {
        continue;
      }  // Continue if Seed not Satisfied
      if ((lookup[centroid][I][0] % 4 == 0) and
          ((Pad2[lookup[centroid][I][0] / 4].Sec.bID >= 0) or
           (Pad2[lookup[centroid][I][0] / 4].Seed.bID % 2 == 1))) {
        continue;
      }  // Continue if Sec is not Expected, and not Empty
      if ((lookup[centroid][I][0] % 4 == 1) and
          ((Pad2[lookup[centroid][I][0] / 4].Sec.bID < 0) or
           (Pad2[lookup[centroid][I][0] / 4].Seed.bID % 2 == 1))) {
        continue;
      }  // Continue if Sec is Expected, and Empty
      if ((lookup[centroid][I][0] % 4 == 2) and
          ((Pad2[lookup[centroid][I][0] / 4].Sec.bID >= 0) or
           (Pad2[lookup[centroid][I][0] / 4].Seed.bID % 2 == 0))) {
        continue;
      }  // Continue if Sec is not Expected, and not Empty
      if ((lookup[centroid][I][0] % 4 == 3) and
          ((Pad2[lookup[centroid][I][0] / 4].Sec.bID < 0) or
           (Pad2[lookup[centroid][I][0] / 4].Seed.bID % 2 == 0))) {
        continue;
      }  // Continue if Sec is Expected, and Empty
      if (not(Pad3[lookup[centroid][I][1] / 4].Seed.Amp > 0)) {
        continue;
      }  // Continue if Seed not Satisfied
      if ((lookup[centroid][I][1] % 4 == 0) and
          ((Pad3[lookup[centroid][I][1] / 4].Sec.bID >= 0) or
           (Pad3[lookup[centroid][I][1] / 4].Seed.bID % 2 == 1))) {
        continue;
      }  // Continue if Sec is not Expected, and not Empty
      if ((lookup[centroid][I][1] % 4 == 1) and
          ((Pad3[lookup[centroid][I][1] / 4].Sec.bID < 0) or
           (Pad3[lookup[centroid][I][1] / 4].Seed.bID % 2 == 1))) {
        continue;
      }  // Continue if Sec is Expected, and Empty
      if ((lookup[centroid][I][1] % 4 == 2) and
          ((Pad3[lookup[centroid][I][1] / 4].Sec.bID >= 0) or
           (Pad3[lookup[centroid][I][1] / 4].Seed.bID % 2 == 0))) {
        continue;
      }  // Continue if Sec is not Expected, and not Empty
      if ((lookup[centroid][I][1] % 4 == 3) and
          ((Pad3[lookup[centroid][I][1] / 4].Sec.bID < 0) or
           (Pad3[lookup[centroid][I][1] / 4].Seed.bID % 2 == 0))) {
        continue;
      }  // Continue if Sec is Expected, and Empty
      cpyCluster(test.Pad2, Pad2[lookup[centroid][I][0] / 4]);
      cpyCluster(test.Pad3, Pad3[lookup[centroid][I][1] / 4]);
      calcResid(test);
      if (test.resid < outTrk[i].resid) {
        cpyTrack(outTrk[i], test);
      }
    }
  }
  // While we ultimately envision having the firmware do duplicate track removal
  // in the other two layers in a separate firmware module, they are done here
  // so as to not have track over counting and to validate the processor. Thats
  // what occurs here below.
  for (int i = 1; i < NTRK - 1; i++) {
    if ((outTrk[i - 1].Pad2.Seed.bID == outTrk[i].Pad2.Seed.bID) and
        (outTrk[i].Pad2.Seed.bID >= 0)) {
      if (outTrk[i - 1].resid <= outTrk[i].resid) {
        clearTrack(outTrk[i]);
      } else {
        clearTrack(outTrk[i - 1]);
      }
    }
    if ((outTrk[i].Pad2.Seed.bID == outTrk[i + 1].Pad2.Seed.bID) and
        (outTrk[i + 1].Pad2.Seed.bID >= 0)) {
      if (outTrk[i + 1].resid <= outTrk[i].resid) {
        clearTrack(outTrk[i]);
      } else {
        clearTrack(outTrk[i + 1]);
      }
    }
    if ((outTrk[i - 1].Pad2.Seed.bID == outTrk[i + 1].Pad2.Seed.bID) and
        (outTrk[i + 1].Pad2.Seed.bID >= 0)) {
      if (outTrk[i - 1].resid <= outTrk[i + 1].resid) {
        clearTrack(outTrk[i + 1]);
      } else {
        clearTrack(outTrk[i - 1]);
      }
    }
  }
  for (int i = 1; i < NTRK - 1; i++) {
    if ((outTrk[i - 1].Pad3.Seed.bID == outTrk[i].Pad3.Seed.bID) and
        (outTrk[i].Pad3.Seed.bID >= 0)) {
      if (outTrk[i - 1].resid <= outTrk[i].resid) {
        clearTrack(outTrk[i]);
      } else {
        clearTrack(outTrk[i - 1]);
      }
    }
    if ((outTrk[i].Pad3.Seed.bID == outTrk[i + 1].Pad3.Seed.bID) and
        (outTrk[i + 1].Pad3.Seed.bID >= 0)) {
      if (outTrk[i + 1].resid <= outTrk[i].resid) {
        clearTrack(outTrk[i]);
      } else {
        clearTrack(outTrk[i + 1]);
      }
    }
    if ((outTrk[i - 1].Pad3.Seed.bID == outTrk[i + 1].Pad3.Seed.bID) and
        (outTrk[i + 1].Pad3.Seed.bID >= 0)) {
      if (outTrk[i - 1].resid <= outTrk[i + 1].resid) {
        clearTrack(outTrk[i + 1]);
      } else {
        clearTrack(outTrk[i - 1]);
      }
    }
  }
  return;
}
