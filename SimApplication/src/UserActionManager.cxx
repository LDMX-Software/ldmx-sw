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

    UserActionManager& UserActionManager::getInstance() { return instance_; }

    actionVec UserActionManager::getActions() { 
        
        if (actions_.empty()) { 
            actions_.push_back(new UserRunAction()); 
            actions_.push_back(new UserEventAction()); 
            actions_.push_back(new UserTrackingAction()); 
            actions_.push_back(new USteppingAction()); 
            actions_.push_back(new UserStackingAction());
        }

        return actions_; 
    }

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

    UserAction* UserActionManager::createAction(const std::string& className, const std::string& instanceName) { 
    
        auto it{actionInfo_.find(className)}; 
        if (it == actionInfo_.end()) return 0; 

        return it->second.builder_(instanceName); 
    }

} // ldmx
