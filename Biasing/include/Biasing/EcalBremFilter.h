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
     * a hard brem within the ecal. 
     *
     * This action is designed similarly to TargetBremFilter, but since
     * the ecal is so much thicker, it is slightly less efficient.
     */
    class EcalBremFilter : public UserAction {

        public:

            /// Constructor
            EcalBremFilter(const std::string& name, Parameters& parameters);

            /// Destructor 
            ~EcalBremFilter() { /* Empty on Purpose */ }

            /**
             * Only continues processing if the step is for the primary track (trackID == 1)
             * and the event hasn't been aborted yet.
             *
             * Checks if this step either stepped from inside CalorimeterRegion to outside
             * or if primary particle stepped from above the brem energy threshold to below.
             * 
             * If either of these things are true, we retrieve the secondaries and check
             * for a secondary that is a product of 'eBrem' and is above the brem energy
             * threshold. For a secondary matching these criteria, we tag it as a brem
             * candidate and increments the count of brem candidates in the user event
             * information.
             *
             * If we aren't able to find at least one brem candidate, we abort the event.
             *
             * @param step The Geant4 step.
             */
            void stepping(const G4Step* step) final override;

            /// Retrieve the type of actions this class defines
            std::vector< TYPE > getTypes() final override { 
                return { TYPE::STEPPING }; 
            } 

        private:

            /// Brem gamma energy treshold [MeV]
            double brem_min_energy_threshold_; 

    }; // EcalBremFilter
}

#endif // BIASING_TARGETBREMFILTER_H
