#ifndef SIMCORE_STEPPRINTER_H
#define SIMCORE_STEPPRINTER_H

#include "SimCore/UserAction.h" 

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
     * User stepping action used to print the details of a step.
     */
    class StepPrinter : public UserAction { 

        public: 

            /** 
             * Constructor.
             *
             * @param[in] name the name of the instance of this UserAction.
             * @param[in] parameters the parameters used to configure this 
             *      UserAction.
             */
            StepPrinter(const std::string& name, Parameters& parameters); 

            /// Destructor
            ~StepPrinter(); 

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

           /// The track ID to filter on
           int trackID_{-9999};  

    }; // StepPrinter

} // ldmx

#endif // SIMCORE_STEPPRINTER_H
