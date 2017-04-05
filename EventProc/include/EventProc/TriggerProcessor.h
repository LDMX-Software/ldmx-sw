/**
 * @file TriggerProcessor.h
 * @brief Class that provides a trigger decision for recon using a TriggerResult object
 * @author Josh Hiltbrand, University of Minnesota
 */

#ifndef EVENTPROC_TRIGGERPROCESSOR_H_
#define EVENTPROC_TRIGGERPROCESSOR_H_

// LDMX
#include "Event/TriggerResult.h"
#include "Framework/EventProcessor.h"

namespace ldmx {

    /**
     * @class TriggerProcessor
     * @brief Provides a trigger decision for recon using a TriggerResult object.
     *
     * @note
     * TriggerProcessor takes in a set of parameters to be used in defining
     * the trigger algorithm. An event is passed to the processor and the relevant
     * algorithms are then run on the event (ECAL layer sum). A trigger decision is
     * executed and the decision along with the algorithm name and relevant variables
     * are stored in a TriggerResult object which is added to the collection.
     */
    class TriggerProcessor : public Producer {

        public:

            /**
             * Class constructor.
             */
            TriggerProcessor(const std::string& name, Process& process) :
                Producer(name, process) {
            }


            /**
             * Class destructor.
             */
            virtual ~TriggerProcessor() {;}

            /**
             * Read in a set of user specified parameters to be used
             * in defining aspects of the trigger.
             * @param pSet A set containing parameters to be used
             * by the trigger algorithm.
             */
            virtual void configure(const ParameterSet& pSet);

            /**
             * Run the trigger algorithm and create a TriggerResult
             * object to contain info about the trigger decision
             * such as pass/fail, number of saved variables,
             * etc.
             * param event The event to run trigger algorithm on.
             */
            virtual void produce(Event& event);

        private:

            /** The energy sum to make cut on. */
            float layerESumCut_{0};

            /** The trigger mode to run in. Mode zero sums over
             * all cells in layer, while in mode 1 only cells in
             * center module are summed over. (TODO)
             */
            int mode_{0};

            /** The first layer of layer sum. */
            int startLayer_{0};

            /** The last layer of layer sum. */
            int endLayer_{0};

            /** The name of the trigger algorithm used. */
            TString algoName_;

            /** Object to hold trigger results and variables */
            TriggerResult result_;

    };

}

#endif
