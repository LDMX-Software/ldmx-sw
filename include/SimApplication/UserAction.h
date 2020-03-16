
#ifndef SIMCORE_USERACTION_H
#define SIMCORE_USERACTION_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <map>
#include <iostream>
#include <string>
#include <vector>

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4UserStackingAction.hh"

// Forward Declarations
class G4Event; 
class G4Run; 
class G4Step;
class G4Track;

namespace ldmx { 

    /// Enum for each of the user action types. 
    enum TYPE { RUN = 1,
                EVENT, 
                TRACKING,  
                STEPPING,
                STACKING, 
                NONE 
    }; 

    // Forward declarations
    class UserAction; 
    
    
    typedef UserAction* UserActionBuilder(const std::string& name); 

    /**
     * @class UserAction
     * @brief Interface that defines a user action.
     */
    class UserAction { 

        public: 

            /**
             * Constructor.
             *
             * @param name Name given the to class instance. 
             */
            UserAction(const std::string& name); 

            /// Destructor
            virtual ~UserAction();

            /**
             * Method used to register a user action with the manager.
             *
             * @param className Name of the class instance
             * @param builder The builder used to create and instance of this class. 
             */
            static void declare(const std::string& className, UserActionBuilder* builder); 

            /// @return The user action types
            virtual std::vector< TYPE > getTypes() = 0; 

        private:

            /// Name of the UserAction
            std::string name_{""}; 

    }; // UserAction

    
    /**
     * @class EventAction
     * @brief Class used to define actions to take at the beginning and end of
     *        an event.
     */
    class EventAction : public UserAction { 
    
        public: 

            /**
             * Constructor.
             *
             * @param name Name given the to class instance. 
             */
            EventAction(const std::string& name);

            /**
             * Method called at the beginning of every event.
             *
             * @param event Geant4 event object.
             */
            virtual void BeginOfEventAction(const G4Event* evemt) = 0; 

            /**
             * Method called at the end of every event.
             *
             * @param event Geant4 event object.
             */
            virtual void EndOfEventAction(const G4Event* event) = 0; 
            
    };

    /**
     * @class RunAction
     * @brief Class used to define actions to take at the beginning and end of
     *        an run.
     */
    class RunAction : public UserAction { 
    
        public: 

            /**
             * Constructor.
             *
             * @param name Name given the to class instance. 
             */
            RunAction(const std::string& name);

            /**
             * Method called at the beginning of a run.
             *
             * @param run Curremt Geant4 run object.
             */
            virtual void BeginOfRunAction(const G4Run* run) = 0; 
            
            /**
             * Method called at the end of a run.
             *
             * @param run Curremt Geant4 run object.
             */
            virtual void EndOfRunAction(const G4Run* run) = 0; 
            
    };

    /**
     * @class TrackingAction
     * @brief Class used to define actions to take when a track is created.
     */
    class TrackingAction : public UserAction { 
    
        public: 

            /**
             * Constructor.
             *
             * @param name Name given the to class instance. 
             */
            TrackingAction(const std::string& name);

            virtual void PreUserTrackingAction(const G4Track* track) = 0; 

            virtual void PostUserTrackingAction(const G4Track* track) = 0; 
            
    };

    /**
     * @class SteppingAction
     * @brief Class used to define actions to take when a track is created.
     */
    class SteppingAction : public UserAction { 

        public:
           
            /**
             * Constructor.
             *
             * @param name Name given the to class instance. 
             */
            SteppingAction(const std::string& name); 

            /**
             *
             */
            virtual void stepping(const G4Step* step) = 0; 

    };

    class StackingAction : public UserAction { 
    
        public: 

            /**
             * Constructor.
             *
             * @param name Name given the to class instance. 
             */
            StackingAction(const std::string& name);

            virtual G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* track) = 0;  

            virtual void NewStage() = 0; 

            virtual void PrepareNewEvent() = 0; 
            
    };


} // ldmx

#define DECLARE_ACTION(NS, CLASS) ldmx::UserAction* CLASS ## Builder (const std::string& name) { return new NS::CLASS(name); } __attribute((constructor(205))) static void CLASS ## Declare() { ldmx::UserAction::declare(std::string(#NS) + "::" + std::string(#CLASS), & CLASS ## Builder); } 

#endif // SIMCORE_USERACTION_H
