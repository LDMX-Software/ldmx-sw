#include "TSTriggerAlgo.hpp"

// Top-level function
// It will run every event and all digis from a single event are passed as an argument
void TSTriggerAlgo(TSDigi digis[TS_PARAM_DIGIS_PER_EVENT], TSTrack tracks[TS_PARAM_MAX_TRACKS]) {
#pragma HLS INTERFACE ap_fifo depth=16 port=digis
#pragma HLS INTERFACE ap_fifo depth=16 port=tracks
#pragma HLS ARRAY_PARTITION variable=digis complete dim=0
#pragma HLS PIPELINE II=5
//#pragma HLS LATENCY max=5

	// Exponent to linear
	uint32_t qie_adc[TS_PARAM_DIGIS_PER_EVENT];
	for (unsigned int i = 0; i < TS_PARAM_DIGIS_PER_EVENT; i++) {
		#pragma HLS UNROLL
		qie_adc[i] = digis[i].getDecodedAdc();
	}

	// Apply energy threshold cut and TDC must be valid
	bool above_thr[TS_PARAM_DIGIS_PER_EVENT];
	for (unsigned int i = 0; i < TS_PARAM_DIGIS_PER_EVENT; i++) {
		#pragma HLS UNROLL
		if (qie_adc[i] > 10000 && digis[i].isTdcValid()) above_thr[i] = true;
		else above_thr[i] = false;
	}

	// Trigger if one QIE is above threshold
	bool filter = false;
	for (unsigned int i = 0; i < TS_PARAM_DIGIS_PER_EVENT; i++) {
		#pragma HLS UNROLL
		filter |= above_thr[i];
	}

	// Dummy code, just fills the variables with some data to generate datapath
	for (unsigned int i = 0; i < TS_PARAM_MAX_TRACKS; i++) {
		#pragma HLS UNROLL
		tracks[i] = TSTrack(i, i, i, i * filter);
	}
}
