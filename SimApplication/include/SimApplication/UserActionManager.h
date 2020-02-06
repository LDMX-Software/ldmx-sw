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

namespace ldmx { 

    // Forward declarations  
    class UserRunAction; 
    class UserEventAction; 
    class UserTrackingAction; 
    class SteppingAction; 
    class UserStackingAction; 

    typedef std::variant < UserRunAction*, 
                           UserEventAction*, 
                           UserTrackingAction*, 
                           SteppingAction*, 
                           UserStackingAction* >   action; 

    typedef std::vector < action > actionVec;  


    /**
     * @class UserActionManager 
     * @brief 
     */
    class UserActionManager { 
    
        public: 

            /**
             * Constructor.
             */
            UserActionManager(); 

            /**
             * Destrcutor.
             */
            ~UserActionManager();

           /**
            *
            */ 
            actionVec getActions() { return actions_; }


            /**
             *
             */
            /*template <class T> 
            T* createUserAction(const std:string& className, const std::string& instanceName) { 
                std::cout << "Class: " << std::endl;
                return nullptr; 
            }*/

        private:

            /// Container for all Geant4 actions
            actionVec actions_;  

    
    };  // UserActionManager 

} // ldmx 

#endif // SIMCORE_USERACTIONMANAGER_H
