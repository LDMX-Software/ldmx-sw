/**
 * @file TrackerHitKiller.h
 * @brief Processor used to drop simulated hits in accordance with the 
 *        expected/observed hit efficiency.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENTPROC_TRACKERHITKILLER_H_
#define EVENTPROC_TRACKERHITKILLER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <time.h>

//----------//
//   ROOT   //
//----------//
#include "TRandom3.h"
#include "TClonesArray.h"

//----------//
//   LDMX   //
//----------//
#include "Event/SimTrackerHit.h"
#include "Event/SiStripHit.h"
#include "Framework/EventProcessor.h"

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
             * @param pSet Set of parameters used to configure this processor.
             */
            void configure(const ParameterSet &pSet);

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

            /** Collection of digitized tracker strip hits. */
            TClonesArray* siStripHits_{nullptr};

            /** 
             * Hit efficiency. For now, this is an integer in the range 0-100.
             */
            int hitEff_{99};

    }; // TrackerHitKiller
}

#endif // EVENTPROC_TRACKERHITKILLER_H_
