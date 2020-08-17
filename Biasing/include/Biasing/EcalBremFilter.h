#ifndef BIASING_ECALBREMFILTER_H
#define BIASING_ECALBREMFILTER_H

//----------------//
//   C++ StdLib   //
//----------------//
#include <algorithm>

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Parameters.h" 

namespace ldmx {

    /**
     * User action that allows a user to filter out events that don't result in
     * a brem within the target. 
     */
    class EcalBremFilter : public UserAction {

        public:

            /// Constructor
            EcalBremFilter(const std::string& name, Parameters& parameters);

            /// Destructor 
            ~EcalBremFilter();

            /**
             * Implement the stepping action which performs the target volume biasing.
             * @param step The Geant4 step.
             */
            void stepping(const G4Step* step) final override;

            /// Retrieve the type of actions this class defines
            std::vector< TYPE > getTypes() final override { 
                return { TYPE::STEPPING }; 
            } 

        private:

            /// Brem gamma energy treshold
            double bremEnergyThreshold_; 

    }; // EcalBremFilter
}

#endif // BIASING_TARGETBREMFILTER_H
