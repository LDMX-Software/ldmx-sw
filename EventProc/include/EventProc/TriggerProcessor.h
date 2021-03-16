/**
 * @file TriggerProcessor.h
 * @brief Class that provides a trigger decision for recon using a TriggerResult object
 * @author Josh Hiltbrand, University of Minnesota
 * @author Lene Kristian Bryngemark, Stanford University
 */

#ifndef EVENTPROC_TRIGGERPROCESSOR_H_
#define EVENTPROC_TRIGGERPROCESSOR_H_

// LDMX
#include "DetDescr/EcalHexReadout.h"
#include "Event/TriggerResult.h"
#include "Ecal/Event/EcalHit.h"
#include "Framework/EventProcessor.h"
#include "Framework/Configure/Parameters.h" 

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
             * Configure the processor using the given user specified parameters.
             * 
             * @param parameters Set of parameters used to configure this processor.
             */
            void configure(Parameters& parameters) final override;

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
       	    std::vector< double > layerESumCuts_;

            /** The beam energy. */
            double beamEnergy_{0.};

            /** The trigger mode to run in. Mode zero sums over
             * all cells in layer, while in mode 1 only cells in
             * center module are summed over. (TODO)
             */
            int mode_{0};

            /** The first layer of layer sum. */
            int startLayer_{0};

            /** The endpoint layer of layer sum: not inclusive -- i.e. this is the first layer not included in the sum. */
            int endLayer_{0};
	  
            /** The name of the trigger algorithm used. */
            TString algoName_;

            /** The name of the input collection (the Ecal hits). */
            std::string inputColl_;

            /** The name of the output collection (the trigger decision). */
            std::string outputColl_;
      
    };

}

#endif
