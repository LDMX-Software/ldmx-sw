/**
 * @file SimEvent.h
 * @brief Class which implements a simulated event with hit and particle collections
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_SIMEVENT_H_
#define EVENT_SIMEVENT_H_

#include "Event/Event.h"
#include "Event/EventConstants.h"

namespace event {

/**
 * @class SimEvent
 * @brief Provides access to simulated event data including hits and SimParticle collection
 */
class SimEvent : public Event {

    public:

        /**
         * Class constructor.
         */
        SimEvent();

        /**
         * Class destructor.
         */
        virtual ~SimEvent() {;}

        /**
         * Get the type of the event (e.g. "SimEvent").
         * @return The type of the event.
         */
        virtual const std::string& getEventType() {
            return event::EventConstants::SIM_EVENT;
        }

        /**
         * Print out summary information about the event.
         */
        void Print(Option_t*) const;

    private:

        /**
         * The collection of SimParticles.
         */
        TClonesArray* simParticles_; //->

        /**
         * The SimTrackerHit collection from the Tagger Tracker.
         */
        TClonesArray* taggerSimHits_; //->

        /**
         * The SimTrackerHit collection from the Recoil Tracker.
         */
        TClonesArray* recoilSimHits_; //->

        /**
         * The SimCalorimeterHit collection from the ECal.
         */
        TClonesArray* ecalSimHits_; //->

        /**
         * The SimCalorimeterHit collection from the HCal.
         */
        TClonesArray* hcalSimHits_; //->

        /**
         * The SimCalorimeterHit collection from the trigger pads.
         */
        TClonesArray* triggerPadSimHits_; //->

        /**
         * The SimCalorimeterHit collection from the target.
         */
        TClonesArray* targetSimHits_; //->

    /**
     * The ROOT class definition.
     */
    ClassDef(SimEvent, 1);
};

}

#endif
