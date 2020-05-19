/**
 * @file TrackerHitKiller.h
 * @brief Processor used to drop simulated hits in accordance with the 
 *        expected/observed hit efficiency.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENTPROC_TRACKERHITKILLER_H
#define EVENTPROC_TRACKERHITKILLER_H

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TRandom3.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/EventProcessor.h"
#include "Framework/Parameters.h" 

namespace ldmx { 

    class TrackerHitKiller : public Producer { 
    
        public: 

            /** Constructor */
            TrackerHitKiller(const std::string &name, Process &process); 
            
            /** Destructor */
            ~TrackerHitKiller();

            /** 
             * Configure the processor using the given user specified parameters.
             * 
             * @param parameters Set of parameters used to configure this processor.
             */
            void configure(Parameters& parameters) final override;

            /**
             * Run the processor and create a collection of "digitized" Si
             * strip hits which include hit efficiency effects.
             *
             * @param event The event to process.
             */
            void produce(Event &event); 

        private: 

            /** 
             * Random number generator used to determine if hit should be
             * dropped. 
             */
            std::unique_ptr<TRandom3> random_;

            /// Hit efficiency
            double hitEff_{99};

    }; // TrackerHitKiller

} // ldmx

#endif // EVENTPROC_TRACKERHITKILLER_H_
