#ifndef BIASING_TRACKPROCESSFILTER_H
#define BIASING_TRACKPROCESSFILTER_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <string>

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimApplication/UserAction.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Parameters.h" 

// Forward declarations
class G4Track; 

namespace ldmx { 

    /**
     * Filter used to tag tracks for persistence based on the process they were
     * created from.
     *
     * All tracks are checked during the post user tracking action stage and 
     * are tagged to be persisted if the process used to create them match the
     * user specified process. The process name specified by the user needs to
     * match the names assigned by Geant4.
     *
     */
    class TrackProcessFilter : public UserAction {

        public: 
        
            /**
             * Constructor.
             *
             * @param[in] name the name of the instance of this UserAction.
             * @param[in] parameters the parameters used to configure this 
             *      UserAction.
             */
            TrackProcessFilter(const std::string& name, Parameters& parameters); 

            /// Destructor 
            ~TrackProcessFilter(); 

            /**
             * Method called after a step has been taken.
             *
             * @param[in] track Geant4 track associated with a particle.
             */
            void PostUserTrackingAction(const G4Track* track) final override;

            /// Retrieve the type of actions this class defines.
            std::vector< TYPE > getTypes() final override { 
                return { TYPE::TRACKING }; 
            } 

        private:      

            /// The process to filter on. 
            std::string process_{""};        
    
    }; // TrackProcessFilter

} // ldmx 

#endif // BIASING_TRACKPROCESSFILTER_H

