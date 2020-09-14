#ifndef BIASING_ECALPROCESSFILTER_H
#define BIASING_ECALPROCESSFILTER_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <algorithm>

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h" 

// Forward declaration
class G4Step; 
class G4Track; 

namespace ldmx {

    /**
     * User action plugin that filters events that don't see a hard brem from 
     * the target undergo a photo-nuclear reaction in the ECal. 
     */
    class EcalProcessFilter : public UserAction {

        public:

            /**
             *
             */
            EcalProcessFilter(const std::string& name, Parameters& parameters);

            /// Destructor
            ~EcalProcessFilter();

            void stepping(const G4Step* step) final override;

            //void PostUserTrackingAction(const G4Track*) final override; 

            /**
             * Classify a new track which postpones track processing.
             * Track processing resumes normally if a target PN interaction occurred.
             * @param aTrack The Geant4 track.
             * @param currentTrackClass The current track classification.
             */
            G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* aTrack, 
                    const G4ClassificationOfNewTrack& currentTrackClass) final override;

            /// Retrieve the type of actions this class defines
            std::vector< TYPE > getTypes() final override { 
                return { TYPE::STACKING, TYPE::STEPPING }; 
            } 

        private:

            /** Pointer to the current track being processed. */
            G4Track* currentTrack_{nullptr};

            /// Process to filter
            std::string process_{""}; 

    }; // EcalProcessFilter 
}

#endif // BIASING_ECALPROCESSFILTER_H
