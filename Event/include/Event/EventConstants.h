#ifndef EVENT_EVENTCONSTANTS_H_
#define EVENT_EVENTCONSTANTS_H_

namespace event {

/*
 * Type names.
 */
static const char* SIM_EVENT = "event::SimEvent";
static const char* SIM_PARTICLE = "event::SimParticle";
static const char* SIM_CALORIMETER_HIT = "event::SimCalorimeterHit";
static const char* SIM_TRACKER_HIT = "event::SimTrackerHit";

/*
 * Collection names.
 */
static const char* SIM_PARTICLES = "SimParticles";
static const char* RECOIL_SIM_HITS = "RecoilSimHits";
static const char* TAGGER_SIM_HITS = "TaggerSimHits";
static const char* ECAL_SIM_HITS = "EcalSimHits";
static const char* HCAL_SIM_HITS = "HcalSimHits";
static const char* TRIGGER_PAD_SIM_HITS = "TriggerPadSimHits";
static const char* TARGET_SIM_HITS = "TargetSimHits";

/*
 * Default collection size for TClonesArray in event objects.
 */
const static int DEFAULT_COLLECTION_SIZE = 100;

}

#endif
