
#include "SimCore/UserAction.h"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserActionManager.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Event.hh"
#include "G4Run.hh"
#include "G4Step.hh"
#include "G4Track.hh"

namespace ldmx { 

    UserAction::UserAction(const std::string& name, Parameters& parameters) { 
        name_ = name; 
        parameters_ = parameters; 
    }

    UserAction::~UserAction() {} 

    void UserAction::declare(const std::string& className, UserActionBuilder* builder) {
        UserActionManager::getInstance().registerAction(className, builder);      
    }
}
