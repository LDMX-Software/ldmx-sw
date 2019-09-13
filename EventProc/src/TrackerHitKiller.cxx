/**
 * @file TrackerHitKiller.cxx
 * @brief Processor used to drop simulated hits in accordance with the 
 *        expected/observed hit efficiency.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "EventProc/TrackerHitKiller.h"

namespace ldmx { 

    TrackerHitKiller::TrackerHitKiller(const std::string& name, Process& process) :
        Producer(name, process) { 

        random_ = std::make_unique<TRandom3>(time(nullptr));
    }
    
    TrackerHitKiller::~TrackerHitKiller() { 
        if ( siStripHits_ ) delete siStripHits_;
    }

    void TrackerHitKiller::configure(const ParameterSet &pSet) { 
       
        // Get the hit efficiency. For now, this will be an integer quantiy 
        // in the range 0-100.  If more precision is needed in the future, 
        // this can be updated.
        hitEff_ = int(pSet.getDouble("hitEfficiency"));

        // Instantiate the collection of Si strip hits 
        siStripHits_ = new TClonesArray("ldmx::SiStripHit", 10000); 
    }

    void TrackerHitKiller::produce(Event& event) { 
       
        // Get the collection of Recoil sim hits from the event 
        const TClonesArray* recoilSimHits = event.getCollection("RecoilSimHits");

        int iHit = 0;
        for (int hitCount = 0; hitCount < recoilSimHits->GetEntriesFast(); ++hitCount) { 
            
            if (random_->Integer(100) >= hitEff_) { 
                std::cout << "[ TrackerHitKiller ]: Dropping hit." << std::endl;
                continue;
            } else {
                // Get the SimTrackerHit from the collection of recoil sim hits.
                SiStripHit* stripHit = static_cast<SiStripHit*>(siStripHits_->ConstructedAt(iHit));
                stripHit->addSimTrackerHit(static_cast<SimTrackerHit*>(recoilSimHits->At(hitCount))); 
                ++iHit;
            }
        }

        //Add the result to the collection
        event.add("SiStripHits", siStripHits_);
    }
}

DECLARE_PRODUCER_NS(ldmx, TrackerHitKiller) 
