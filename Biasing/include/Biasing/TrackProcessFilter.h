/**
 * @file TrackProcessFilter.cxx
 * @brief Filter used to flag tracks for saving based on the process they 
 *        were created from.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

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

    class TrackProcessFilter : public UserAction {

        public: 
        
            /**
             *
             */
            TrackProcessFilter(const std::string& name, Parameters& parameters); 

            /// Destructor 
            ~TrackProcessFilter(); 

            /**
             *
             */
            void PostUserTrackingAction(const G4Track* track) final override;

            /// Retrieve the type of actions this class defines
            std::vector< TYPE > getTypes() final override { 
                return { TYPE::TRACKING }; 
            } 

        private:      

            std::vector < std::string > process_;        
    
    }; // TrackProcessFilter

} // ldmx 

#endif // BIASING_TRACKPROCESSFILTER_H

