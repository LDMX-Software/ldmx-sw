/**
 * @file FindableTrackResult.h
 * @brief Class used to encapsulate the results obtained from 
 *        FindableTrackProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Event/FindableTrackResult.h"

ClassImp(ldmx::FindableTrackResult)

namespace ldmx {
            
    FindableTrackResult::FindableTrackResult() { }

    FindableTrackResult::~FindableTrackResult() {}

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

    void FindableTrackResult::Clear() { 
        particleTrackID_= -1;
        is4sFindable_   = false; 
        is3s1aFindable_ = false;
        is2s2aFindable_ = false;
        is2aFindable_   = false; 
        is2sFindable_   = false;
        is3sFindable_   = false;
    }

    void FindableTrackResult::Print(std::ostream& o) const { 
        o << "FindableTrackResult {"
          << "SimParticle Track ID: " 
          << particleTrackID_ << std::boolalpha
          << ", 4s Findable: "   << is4sFindable_
          << ", 3s1a Findable: " << is3s1aFindable_
          << ", 2s2a Findable: " << is2s2aFindable_
          << ", 2a Findable: "   << is2aFindable_
          << ", 2s Findable: "   << is2sFindable_
          << ", 3s Findable: "   << is3sFindable_
          << "}";
    }
}
