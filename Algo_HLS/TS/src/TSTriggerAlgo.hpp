#ifndef __TS_TRIGGER_TOP_HPP__
#define __TS_TRIGGER_TOP_HPP__

#include "TSTriggerParameters.hpp"
#include "TSDigi.hpp"
#include "TSTrack.hpp"

void TSTriggerAlgo(TSDigi digis[TS_PARAM_DIGIS_PER_EVENT], TSTrack tracks[TS_PARAM_MAX_TRACKS]);

#endif /* !__TS_TRIGGER_TOP_HPP__ */
