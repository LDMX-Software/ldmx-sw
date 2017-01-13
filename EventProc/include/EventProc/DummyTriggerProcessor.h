/**
 * @file ExampleCalHitProcessor.h
 * @brief Class containing an example SimCalorimeterHit processor
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENTPROC_DUMMYTRIGGERPROCESSOR_H_
#define EVENTPROC_DUMMYTRIGGERPROCESSOR_H_

// LDMX
#include "Event/EventConstants.h"
#include "Event/TriggerResult.h"

namespace eventproc {

/**
 * @class DummyTriggerProcessor
 * @brief Dummy event processor which adds trigger results to output
 */
class DummyTriggerProcessor : public EventProcessor {

    public:

        void initialize() {
            trig_.set("DUMMY_TRIG", false, 1);
        }

        void execute() {

            int eventNumber = getEvent()->getEventHeader("sim")->getEventNumber();

            if (eventNumber % 2 == 0) {
                trig_.set("DUMMY_TRIG", true, 1);
            } else {
                trig_.set("DUMMY_TRIG", false, 1);
            }

            trig_.setAlgoVar(0, eventNumber);

            event_->add(event::EventConstants::TRIGGER_RESULT, &trig_);
        }

        void finish() {
        }

    public:
        event::TriggerResult trig_;
};

}

#endif /* EVENTPROC_DUMMYTRIGGERPROCESSOR_H_ */
