#ifndef BIASING_TAGGERVETOFILTER_H
#define BIASING_TAGGERVETOFILTER_H

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
     * User stepping action used to filter events that see the incident electron
     * fall below an energy threshold before reaching the target.
     *
     */
    class TaggerVetoFilter : public UserAction { 
    
        public: 
            
            /**
             * Constructor.
             *
             * @param[in] name the name of the instance of this UserAction.
             * @param[in] parameters the parameters used to configure this 
             *      UserAction.
             */
            TaggerVetoFilter(const std::string& name, Parameters& parameters); 

            /// Destructor
            ~TaggerVetoFilter();

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

            /// Energy below which an incident electron should be vetoed.
            double threshold_{0}; 

    };  // TaggerVetoFilter

} // ldmx

#endif // BIASING_TAGGERVETOFILTER_H 
