#ifndef EVENT_SIMEVENT_H_
#define EVENT_SIMEVENT_H_

#include "Event/Event.h"
#include "Event/EventConstants.h"

namespace event {

class SimEvent : public Event {

    public:

        SimEvent();

        virtual ~SimEvent() {;}

        virtual const std::string& getEventType() {
            return event::EventConstants::SIM_EVENT;
        }

        void Print(Option_t*) const;

    private:

        TClonesArray* simParticles_; //->
        TClonesArray* taggerSimHits_; //->
        TClonesArray* recoilSimHits_; //->
        TClonesArray* ecalSimHits_; //->
        TClonesArray* hcalSimHits_; //->
        TClonesArray* triggerPadSimHits_; //->
        TClonesArray* targetSimHits_; //->

        ClassDef(SimEvent, 1);
};

}

#endif
