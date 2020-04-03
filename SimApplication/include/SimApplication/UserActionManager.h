/**
 *
 */

#ifndef SIMCORE_USERACTIONMANAGER_H
#define SIMCORE_USERACTIONMANAGER_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <memory> 
#include <string> 
#include <variant>
#include <vector> 

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Parameters.h" 

#include "SimApplication/UserRunAction.h" 
#include "SimApplication/UserEventAction.h" 
#include "SimApplication/UserTrackingAction.h" 
#include "SimApplication/USteppingAction.h" 
#include "SimApplication/UserStackingAction.h" 

#include "SimApplication/UserAction.h" 

namespace ldmx { 


    typedef std::variant < UserRunAction*, 
                           UserEventAction*, 
                           UserTrackingAction*, 
                           USteppingAction*, 
                           UserStackingAction* >   action; 

    typedef std::map < TYPE, action > actionMap;  


    /**
     * @class UserActionManager 
     * @brief 
     */
    class UserActionManager { 
    
        public:

            /// @return the UserActionManager instance 
            static UserActionManager& getInstance(); 

           /**
            *
            */ 
            actionMap getActions(); 

            /**
             *
             */
            void registerAction(const std::string& className, UserActionBuilder* builder);

           /**
            *
            */
            void createAction(const std::string& className, const std::string& instanceName, Parameters& parameters); 

        private:

            /// UserActionManager instance 
            static UserActionManager instance_; 

            /// Container for all Geant4 actions
            actionMap actions_; 


            /**
              * @struct ActionInfo
              * @brief Encapsulates the information required to create a UserAction
              */
            struct ActionInfo { 

                /// Name of the class
                std::string className_; 

                /// Class builder
                UserActionBuilder* builder_; 
            };

            /// A map of all registered user actions to their corresponding info.
            std::map < std::string, ActionInfo > actionInfo_; 
            
            /// Private constructor to prevent instatiation 
            UserActionManager(); 

    };  // UserActionManager 

} // ldmx 

#endif // SIMCORE_USERACTIONMANAGER_H
