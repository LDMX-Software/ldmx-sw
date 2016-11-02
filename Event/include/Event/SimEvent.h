#ifndef EVENT_SIMEVENT_H_
#define EVENT_SIMEVENT_H_

#include "Event/Event.h"

namespace event {

class SimEvent : public Event {

    public:

        SimEvent();

        virtual ~SimEvent() {;}

        virtual const char* getEventType() {
            return event::SIM_EVENT;
        }

    private:

        TClonesArray* simParticles;
        TClonesArray* taggerSimHits;
        TClonesArray* recoilSimHits;
        TClonesArray* ecalSimHits;
        TClonesArray* hcalSimHits;
        TClonesArray* triggerPadSimHits;
        TClonesArray* targetSimHits;

        ClassDef(SimEvent, 1);
};

}

#endif
