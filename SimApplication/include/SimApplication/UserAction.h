/**
 *
 */

#ifndef SIMCORE_USERACTION_H
#define SIMCORE_USERACTION_H

#include <map>
#include <iostream>
#include <string> 

class G4Step; 

namespace ldmx { 


    class UserAction; 

    typedef UserAction* UserActionBuilder(const std::string& name); 

    /**
     * @class UserAction
     * @brief Interface that defines a user action plugin.
     */
    class UserAction { 

        public: 

            UserAction(const std::string& name); 

            /**
             * Destructor.
             */
            virtual ~UserAction();

            /**
             *
             */
            static void declare(const std::string& className, UserActionBuilder* builder);  

        private:

            /// Name of the UserAction
            std::string name_{""};  

    }; // UserAction

    /**
     *
     */
    class SteppingAction : public UserAction { 

        public:
           
            /**
             *
             */
            SteppingAction(const std::string& name); 

            /**
             *
             */
            virtual void stepping(const G4Step* step) = 0;  

    };

} // ldmx

#define DECLARE_STEPPING_ACTION(NS, CLASS) ldmx::UserAction* CLASS ## Builder (const std::string& name) { return new NS::CLASS(name); } __attribute((constructor(205))) static void CLASS ## Declare() { ldmx::UserAction::declare(std::string(#NS) + "::" + std::string(#CLASS), & CLASS ## Builder); } 

#endif // SIMCORE_USERACTION_H
