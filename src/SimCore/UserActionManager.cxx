/**
 *
 */

#include "SimCore/UserActionManager.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <dlfcn.h> 

#include "Framework/Exception/Exception.h" 


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

    void UserActionManager::createAction(const std::string& className, const std::string& instanceName, Parameters& parameters) { 
        auto it{actionInfo_.find(className)}; 
        if (it == actionInfo_.end()) {
            EXCEPTION_RAISE("UserActionException", "Failed to create " + className); 
        } 

        auto act{it->second.builder_(instanceName, parameters)};
        
        std::vector< TYPE > types = act->getTypes();
        for (auto& type : types) {
            
            if (type == TYPE::RUN) 
                std::get< UserRunAction* >(actions_[TYPE::RUN])->registerAction(act); 
            else if (type == TYPE::EVENT) 
                std::get< UserEventAction* >(actions_[TYPE::EVENT])->registerAction(act); 
            else if (type == TYPE::TRACKING) 
                std::get< UserTrackingAction* >(actions_[TYPE::TRACKING])->registerAction(act); 
            else if (type == TYPE::STEPPING) 
                std::get< USteppingAction* >(actions_[TYPE::STEPPING])->registerAction(act); 
            else if (type == TYPE::STACKING) 
                std::get< UserStackingAction* >(actions_[TYPE::STACKING])->registerAction(act); 
            else 
               EXCEPTION_RAISE("UserActionException", "User action type doesn't exist."); 
        } 
    }

} // ldmx
