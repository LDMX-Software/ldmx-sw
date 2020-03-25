
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
             * @param event Geant4 event object.
             */
            virtual void BeginOfEventAction(const G4Event* evemt) {}; 

            /**
             * Method called at the end of every event.
             *
             * @param event Geant4 event object.
             */
            virtual void EndOfEventAction(const G4Event* event) {};

            /**
             * Method called at the beginning of a run.
             *
             * @param run Curremt Geant4 run object.
             */
            virtual void BeginOfRunAction(const G4Run* run) {}; 
            
            /**
             * Method called at the end of a run.
             *
             * @param run Curremt Geant4 run object.
             */
            virtual void EndOfRunAction(const G4Run* run) {};


            virtual void PreUserTrackingAction(const G4Track* track) {}; 

            virtual void PostUserTrackingAction(const G4Track* track) {};

            /**
             *
             */
            virtual void stepping(const G4Step* step) {};

            virtual G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* track, const G4ClassificationOfNewTrack& currentTrackClass ) {};  

            virtual void NewStage() {}; 

            virtual void PrepareNewEvent() {}; 

            /// @return The user action types
            virtual std::vector< TYPE > getTypes() = 0; 

        protected:

            /// Name of the UserAction
            std::string name_{""};

            
            /// The set of parameters used to configure this class
            Parameters parameters_; 

    }; // UserAction

} // ldmx

#define DECLARE_ACTION(NS, CLASS) ldmx::UserAction* CLASS ## Builder (const std::string& name, ldmx::Parameters& parameters) { return new NS::CLASS(name, parameters); } __attribute((constructor(205))) static void CLASS ## Declare() { ldmx::UserAction::declare(std::string(#NS) + "::" + std::string(#CLASS), & CLASS ## Builder); } 

#endif // SIMCORE_USERACTION_H
