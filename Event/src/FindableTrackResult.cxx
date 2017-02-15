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

    void FindableTrackResult::setResult(SimParticle* simParticle, bool isFindable) { 
        simParticle_ = simParticle; 
        isFindable_ = isFindable;
    }

    void FindableTrackResult::Print(Option_t *option) { 
        std::cout << "[ FindableTrackResult ]: "
                  << "Sim particle PDG ID: " 
                  << static_cast<SimParticle*>(simParticle_.GetObject())->getPdgID() 
                  << " is Findable: " << isFindable_ << std::endl;
    }
}
