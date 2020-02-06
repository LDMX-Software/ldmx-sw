/**
 *
 */

#include "SimApplication/UserActionManager.h"

#include "SimApplication/UserRunAction.h" 
#include "SimApplication/UserEventAction.h" 
#include "SimApplication/UserTrackingAction.h" 
#include "SimApplication/SteppingAction.h" 
#include "SimApplication/UserStackingAction.h" 


namespace ldmx {
    
    UserActionManager::UserActionManager() {
        
        actions_.push_back(new UserRunAction); 
        actions_.push_back(new UserEventAction); 
        actions_.push_back(new UserTrackingAction); 
        actions_.push_back(new SteppingAction); 
        actions_.push_back(new UserStackingAction);  
    }

    UserActionManager::~UserActionManager() {
    
        std::for_each( actions_.begin(), actions_.end(), 
                [](action& a) { delete std::get<0>(a); } );
        actions_.clear();  
    }

} // ldmx
