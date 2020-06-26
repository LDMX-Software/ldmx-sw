
#ifndef SIMAPPLICATION_USERACTION_H
#define SIMAPPLICATION_USERACTION_H

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

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Parameters.h" 

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
    
    typedef UserAction* UserActionBuilder(const std::string& name, Parameters& parameters); 

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
            UserAction(const std::string& name, Parameters& parameters); 

            /// Destructor
            virtual ~UserAction();

            /**
             * Method used to register a user action with the manager.
             *
             * @param className Name of the class instance
             * @param builder The builder used to create and instance of this class. 
             */
            static void declare(const std::string& className, UserActionBuilder* builder); 

            /**
             * Method called at the beginning of every event.
             *
             * TYPE::EVENT
             *
             * @param event Geant4 event object.
             */
            virtual void BeginOfEventAction(const G4Event* ) {}; 

            /**
             * Method called at the end of every event.
             *
             * TYPE::EVENT
             *
             * @param event Geant4 event object.
             */
            virtual void EndOfEventAction(const G4Event* ) {};

            /**
             * Method called at the beginning of a run.
             *
             * TYPE::RUN
             *
             * @param run Current Geant4 run object.
             */
            virtual void BeginOfRunAction(const G4Run* ) {}; 
            
            /**
             * Method called at the end of a run.
             *
             * TYPE::RUN
             *
             * @param run Current Geant4 run object.
             */
            virtual void EndOfRunAction(const G4Run* ) {};

            /**
             * Method called before the UserTrackingAction.
             *
             * TYPE::TRACKING
             *
             * @param track current Geant4 track
             */
            virtual void PreUserTrackingAction(const G4Track* ) {}; 

            /**
             * Method called after the UserTrackingAction.
             *
             * TYPE::TRACKING
             *
             * @param track current Geant4 track
             */
            virtual void PostUserTrackingAction(const G4Track* ) {};

            /**
             * Method called after each simulation step.
             *
             * TYPE::STEPPING
             *
             * @param current Geant4 step
             */
            virtual void stepping(const G4Step* ) {};

            /**
             * Method called when a track is updated
             *
             * TYPE::STEPPING
             *
             * @param current Geant4 track
             * @param current tracks' classification
             */
            virtual G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* , const G4ClassificationOfNewTrack& cl) { return cl; };

            /**
             * Method called at the beginning of a new stage
             *
             * TYPE::STACKING
             */
            virtual void NewStage() {}; 

            /**
             * Method called at the beginning of a new event
             *
             * TYPE::STACKING
             */
            virtual void PrepareNewEvent() {}; 

            /**
             * @return The user action types
             *
             * Must be defined by any UserActions so that we know what functions to call.
             */
            virtual std::vector< TYPE > getTypes() = 0; 

        protected:

            /// Name of the UserAction
            std::string name_{""};

            
            /// The set of parameters used to configure this class
            Parameters parameters_; 

    }; // UserAction

} // ldmx

#define DECLARE_ACTION(NS, CLASS) ldmx::UserAction* CLASS ## Builder (const std::string& name, ldmx::Parameters& parameters) { return new NS::CLASS(name, parameters); } __attribute((constructor(205))) static void CLASS ## Declare() { ldmx::UserAction::declare(std::string(#NS) + "::" + std::string(#CLASS), & CLASS ## Builder); } 

#endif // SIMAPPLICATION_USERACTION_H
