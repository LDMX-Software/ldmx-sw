
#include "SimApplication/UserAction.h"

#include "SimApplication/UserActionManager.h"

namespace ldmx { 

    UserAction::UserAction(const std::string& name) { 
        name_ = name; 
    }

    UserAction::~UserAction() {} 

    void UserAction::declare(const std::string& className, UserActionBuilder* builder) {
        UserActionManager::getInstance().registerAction(className, builder);      
    }

    SteppingAction::SteppingAction(const std::string& name) : UserAction(name) {}

}
