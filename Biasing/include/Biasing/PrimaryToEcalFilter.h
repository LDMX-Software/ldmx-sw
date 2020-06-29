#ifndef BIASING_PRIMARYTOECALFILTER_H
#define BIASING_PRIMARYTOECALFILTER_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <string>

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Parameters.h" 

// Forward declarations
class G4Step;

namespace ldmx { 

    /**
     * User stepping action used to filter events where the primary
     * particle falls below a threshold before reaching the CalorimeterRegion
     */
    class PrimaryToEcalFilter : public UserAction { 
    
        public: 
            
            /**
             * Constructor.
             *
             * @param[in] name the name of the instance of this UserAction.
             * @param[in] parameters the parameters used to configure this 
             *      UserAction.
             */
            PrimaryToEcalFilter(const std::string& name, Parameters& parameters); 

            /// Destructor
            ~PrimaryToEcalFilter();

            /**
             * Stepping action called when a step is taken during tracking of 
             * a particle. 
             *
             * @param[in] step Geant4 step
             */
            void stepping(const G4Step* step) final override;  

            /// Retrieve the type of actions this class defines
            std::vector< TYPE > getTypes() final override { 
                return { TYPE::STEPPING }; 
            } 
            
        private: 

            /// Energy [MeV] below which a primary should be vetoed.
            double threshold_; 

    };  // PrimaryToEcalFilter

} // ldmx

#endif // BIASING_TAGGERVETOFILTER_H 
