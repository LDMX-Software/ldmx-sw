/**
 *
 */

#include "SimApplication/UserActionManager.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <dlfcn.h> 

#include "Exception/Exception.h" 


ldmx::UserActionManager ldmx::UserActionManager::instance_ __attribute__((init_priority(200))); 

namespace ldmx {

    UserActionManager::UserActionManager() { }

    actionMap UserActionManager::getActions() { 
        
        if (actions_.empty()) {
            actions_[TYPE::RUN]      = new UserRunAction(); 
            actions_[TYPE::EVENT]    = new UserEventAction(); 
            actions_[TYPE::TRACKING] = new UserTrackingAction(); 
            actions_[TYPE::STEPPING] = new USteppingAction();
            actions_[TYPE::STACKING] = new UserStackingAction();   
        }

        return actions_; 
    }

    UserActionManager& UserActionManager::getInstance() { return instance_; }

    void UserActionManager::registerAction(const std::string& className, UserActionBuilder* builder) { 
    
        auto it{actionInfo_.find(className)}; 
        if (it != actionInfo_.end()) { 
            EXCEPTION_RAISE("ExistingEventProcessorDefinition", "The user action " + className + " has already been registered."); 
        }

        ActionInfo info; 
        info.className_ = className;
        info.builder_ = builder;
        
        actionInfo_[className] = info; 
    }

    void UserActionManager::createAction(const std::string& className, const std::string& instanceName) { 
    
        auto it{actionInfo_.find(className)}; 
        if (it == actionInfo_.end()) {
            EXCEPTION_RAISE("UserActionException", "Failed to create " + className); 
        } 

        auto act{it->second.builder_(instanceName)};

        if (act->getType() == TYPE::RUN) 
            std::get< UserRunAction* >(actions_[act->getType()])->registerAction(static_cast<RunAction*>(act)); 
        else if (act->getType() == TYPE::EVENT) 
            std::get< UserEventAction* >(actions_[act->getType()])->registerAction(static_cast<EventAction*>(act)); 
        else if (act->getType() == TYPE::TRACKING) 
            std::get< UserTrackingAction* >(actions_[act->getType()])->registerAction(static_cast<TrackingAction*>(act)); 
        else if (act->getType() == TYPE::STEPPING) 
            std::get< USteppingAction* >(actions_[act->getType()])->registerAction(static_cast<SteppingAction*>(act)); 
        else if (act->getType() == TYPE::STACKING) 
            std::get< UserStackingAction* >(actions_[act->getType()])->registerAction(static_cast<StackingAction*>(act)); 
        else {
            EXCEPTION_RAISE("UserActionException", "Action type  doesn't exist."); 
        }
    }

} // ldmx
