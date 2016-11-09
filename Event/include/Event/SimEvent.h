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

        TClonesArray* simParticles_;
        TClonesArray* taggerSimHits_;
        TClonesArray* recoilSimHits_;
        TClonesArray* ecalSimHits_;
        TClonesArray* hcalSimHits_;
        TClonesArray* triggerPadSimHits_;
        TClonesArray* targetSimHits_;

        ClassDef(SimEvent, 1);
};

}

#endif
