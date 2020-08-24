/**
 * @file TrackerHitKiller.cxx
 * @brief Processor used to drop simulated hits in accordance with the 
 *        expected/observed hit efficiency.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "EventProc/TrackerHitKiller.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <time.h>

/*~~~~~~~~~~~*/
/*   Event   */
/*~~~~~~~~~~~*/
#include "SimCore/SimTrackerHit.h"
#include "Event/SiStripHit.h"

namespace ldmx { 

    TrackerHitKiller::TrackerHitKiller(const std::string& name, Process& process) :
        Producer(name, process) { 

        random_ = std::make_unique<TRandom3>(time(nullptr));
    }
    
    TrackerHitKiller::~TrackerHitKiller() { }

    void TrackerHitKiller::configure(Parameters& parameters) {
       
        // Get the hit efficiency.
        hitEff_ = parameters.getParameter< double >("hitEfficiency");
    }

    void TrackerHitKiller::produce(Event& event) { 
       
        // Get the collection of Recoil sim hits from the event 
        auto recoilSimHits{event.getCollection<SimTrackerHit>("RecoilSimHits")};

        std::vector<SiStripHit> siStripHits;
        for ( const auto& simHit : recoilSimHits ) {
            
            if (random_->Uniform(0, 100) >= hitEff_) continue;
            else {
                // Get the SimTrackerHit from the collection of recoil sim hits.
                siStripHits.emplace_back();
                siStripHits.back().addSimTrackerHit( simHit );
            }
        }

        //Add the result to the collection
        event.add( "SiStripHits", siStripHits );
    }
}

DECLARE_PRODUCER_NS(ldmx, TrackerHitKiller) 
