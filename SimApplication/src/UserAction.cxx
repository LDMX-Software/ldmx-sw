
#include "SimApplication/UserAction.h"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimApplication/UserActionManager.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Event.hh"
#include "G4Run.hh"
#include "G4Step.hh"
#include "G4Track.hh"

namespace ldmx { 

    UserAction::UserAction(const std::string& name) { 
        name_ = name; 
    }

    UserAction::~UserAction() {} 

    void UserAction::declare(const std::string& className, UserActionBuilder* builder) {

        std::cout << "UserAction::declare : Declaring class " << className << std::endl;
        UserActionManager::getInstance().registerAction(className, builder);      
    }
}
