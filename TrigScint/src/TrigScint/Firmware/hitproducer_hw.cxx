#include <stdio.h>

#include <iostream>

#include "TrigScint/Firmware/hitproducer.h"
#include "TrigScint/Firmware/objdef.h"

void hitproducer_hw(ap_uint<14> FIFO[NHITS][5], Hit outHit[NHITS],
                    ap_uint<8> Peds[NHITS]) {
#pragma HLS ARRAY_PARTITION variable = FIFO complete
#pragma HLS ARRAY_PARTITION variable = amplitude complete
#pragma HLS ARRAY_PARTITION variable = Peds complete
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[0]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[1]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[2]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[3]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[4]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[5]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[6]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[7]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[8]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[9]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[10]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[11]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[12]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[13]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[14]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[15]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[16]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[17]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[18]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[19]

#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[20]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[21]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[22]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[23]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[24]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[25]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[26]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[27]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[28]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[29]

#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[30]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[31]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[32]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[33]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[34]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[35]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[36]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[37]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[38]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[39]

#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[40]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[41]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[42]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[43]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[44]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[45]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[46]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[47]

#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[48]
#pragma HLS INTERFACE ap_fifo depth = 16 port = FIFO[49]

#pragma HLS PIPELINE

  // The QIE11 card takes an analogue SiPM PE count
  // and converts electron counts from it via a piecewise
  // exponential curve into an ADC. Depending on the shunts
  // you use, you can affect the gain; the gains and variable
  // values determined here are motived primarily by those required
  // to get the MIP distribution seen in the 2022 beam.
  // The next variables show where each linear portion of the
  // exponential map start in charge count (edges_) and their slope;
  // the hitmaker delinearized the adc counts, integrates over five clockcycles
  // and forms a hit.

  /// Indices of first bin of each subrange
  ap_uint<14> nbins_[5] = {0, 16, 36, 57, 64};

  /// Charge lower limit of all the 16 subranges
  ap_uint<14> edges_[17] = {0,     34,    158,    419,    517,   915,
                            1910,  3990,  4780,   7960,   15900, 32600,
                            38900, 64300, 128000, 261000, 350000};
  /// sensitivity of the subranges (Total charge/no. of bins)
  ap_uint<14> sense_[16] = {3,   6,   12,  25,   25,   50,   99,   198,
                            198, 397, 794, 1587, 1587, 3174, 6349, 12700};

  for (int i = 0; i < NHITS; i++) {
    outHit[i].bID = -1;
    outHit[i].mID = 0;
    outHit[i].Time = 0;
    outHit[i].Amp = 0;
    ap_uint<14> word1 = FIFO[i][0];
    ap_uint<14> word2 = FIFO[i][1];
    ap_uint<14> word3 = FIFO[i][2];
    ap_uint<14> word4 = FIFO[i][3];
    ap_uint<14> word5 = FIFO[i][4];
    ap_uint<16> charge1;
    ap_uint<16> charge2;
    ap_uint<16> charge3;
    ap_uint<16> charge4;
    ap_uint<16> charge5;
    ap_uint<4> shunt = 1;
    // An identical procedure is used for all 5 clockcylces. Namely you extract
    // the adc value from the adc+tdc concatenated value you get from the raw
    // strwam via (word1>>6); You then use what integer multiple of 64 it is to
    // determine which linear segment you are on, and v1 (the remainder) to
    // determine how far along that linear segment your charge carried you.
    // Together that gets you charge.

    ap_uint<14> rr = (word1 >> 6) / 64;
    ap_uint<14> v1 = (word1 >> 6) % 64;
    ap_uint<14> ss =
        1 * (v1 > nbins_[1]) + 1 * (v1 > nbins_[2]) + 1 * (v1 > nbins_[3]);
    charge1 = edges_[4 * rr + ss] + (v1 - nbins_[ss]) * sense_[4 * rr + ss] +
              sense_[4 * rr + ss] / 2 - 1;

    rr = (word2 >> 6) / 64;
    v1 = (word2 >> 6) % 64;
    ss = 1 * (v1 > nbins_[1]) + 1 * (v1 > nbins_[2]) + 1 * (v1 > nbins_[3]);
    charge2 = edges_[4 * rr + ss] + (v1 - nbins_[ss]) * sense_[4 * rr + ss] +
              sense_[4 * rr + ss] / 2 - 1;

    rr = (word3 >> 6) / 64;
    v1 = (word3 >> 6) % 64;
    ss = 1 * (v1 > nbins_[1]) + 1 * (v1 > nbins_[2]) + 1 * (v1 > nbins_[3]);
    charge3 = edges_[4 * rr + ss] + (v1 - nbins_[ss]) * sense_[4 * rr + ss] +
              sense_[4 * rr + ss] / 2 - 1;

    rr = (word4 >> 6) / 64;
    v1 = (word4 >> 6) % 64;
    ss = 1 * (v1 > nbins_[1]) + 1 * (v1 > nbins_[2]) + 1 * (v1 > nbins_[3]);
    charge4 = edges_[4 * rr + ss] + (v1 - nbins_[ss]) * sense_[4 * rr + ss] +
              sense_[4 * rr + ss] / 2 - 1;

    rr = (word5 >> 6) / 64;
    v1 = (word5 >> 6) % 64;
    ss = 1 * (v1 > nbins_[1]) + 1 * (v1 > nbins_[2]) + 1 * (v1 > nbins_[3]);
    charge5 = edges_[4 * rr + ss] + (v1 - nbins_[ss]) * sense_[4 * rr + ss] +
              sense_[4 * rr + ss] / 2 - 1;

    outHit[i].bID = i;

    // You now are creating an output hit. The time of the hit is determined by
    // the last part of the concatenated streamed tdc, which is 6 bits and
    // therefore you mask the word1 with 63 (which is 111111 in binary) so as
    // only to keep the tdc.

    outHit[i].Time = (word1 & 63);

    // The 36 remaining here is an artefact of the mapping that the charges have
    // to adcs; its not particularly meaningful except that it establishes that
    // 0 adc corresponds to 0 charge. The .00625 value is a value which is
    // conglomerate but relates to the number of PE's produced; it will change
    // based on the number of shunts employed during a run.

    outHit[i].Amp =
        shunt *
        ((charge1 + charge2 + charge3 + charge4 + charge5 - 36) * .00625);
  }

  return;
}
