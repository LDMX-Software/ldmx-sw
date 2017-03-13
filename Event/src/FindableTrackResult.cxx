/**
 * @file FindableTrackResult.h
 * @brief Class used to encapsulate the results obtained from 
 *        FindableTrackProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Event/FindableTrackResult.h"

ClassImp(ldmx::FindableTrackResult)

namespace ldmx {
            
    FindableTrackResult::FindableTrackResult() :
        TObject() {  
    }

    FindableTrackResult::~FindableTrackResult() {
    }

    void FindableTrackResult::setResult(Strategy strategy, bool isFindable) { 
        
        switch (strategy) { 
            case FindableTrackResult::STRATEGY_NONE: 
                break;
            case FindableTrackResult::STRATEGY_4S: 
                is4sFindable_ = isFindable;
                break;
            case FindableTrackResult::STRATEGY_3S1A: 
                is3s1aFindable_ = isFindable;
                break;
            case FindableTrackResult::STRATEGY_2S2A: 
                is2s2aFindable_ = isFindable;
                break;
            case FindableTrackResult::STRATEGY_2A:
                is2aFindable_ = isFindable; 
                break;
            case FindableTrackResult::STRATEGY_2S:
                is2sFindable_ = isFindable; 
                break;
            case FindableTrackResult::STRATEGY_3S:
                is3sFindable_ = isFindable; 
                break;
        }
    }

    void FindableTrackResult::Clear(Option_t *option) { 
        simParticle_    = nullptr;
        is4sFindable_   = false; 
        is3s1aFindable_ = false;
        is2s2aFindable_ = false;
        is2aFindable_   = false; 
        is2sFindable_   = false;
        is3sFindable_   = false;
    }

    void FindableTrackResult::Print(Option_t *option) { 
        std::cout << "[ FindableTrackResult ]: "
                  << "Sim particle PDG ID: " 
                  << static_cast<SimParticle*>(simParticle_.GetObject())->getPdgID() << "\n" 
                  << "\t4s Findable: "   << is4sFindable_    << "\n" 
                  << "\t3s1a Findable: " << is3s1aFindable_  << "\n"
                  << "\t2s2a Findable: " << is2s2aFindable_  << "\n"
                  << "\t2a Findable: "   << is2aFindable_  << "\n"
                  << "\t2s Findable: "   << is2sFindable_  << "\n"
                  << "\t3s Findable: "   << is3sFindable_  << "\n"
                  << std::endl;
    }
}
