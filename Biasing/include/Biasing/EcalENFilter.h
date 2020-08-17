#ifndef BIASING_ECALENFILTER_H
#define BIASING_ECALENFILTER_H

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

// Forward Declarations 
class G4Step; 
class G4Event; 

namespace ldmx {

    class EcalENFilter : public UserAction {

        public:

            /**
             * Class constructor.
             */
            EcalENFilter(const std::string& name, Parameters& parameters);

            /// Destructor
            ~EcalENFilter();

            /**
             * Implement the stepping action which performs the ecal volume filtering.
             * @param step The Geant4 step.
             */
            void stepping(const G4Step* step) final override;

            /// Retrieve the type of actions this class defines
            std::vector< TYPE > getTypes() final override { 
                return { TYPE::STEPPING }; 
            } 

        private:

            /** Energy that the recoil electron must not surpass. */
            double recoilEnergyThreshold_;

    }; // EcalENFilter

} // ldmx

#endif // BIASING_TARGETPROCESSFILTER_H
