#ifndef BIASING_TARGETPROCESSFILTER_H
#define BIASING_TARGETPROCESSFILTER_H

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Parameters.h" 

// Forward declaration
class G4Event; 
class G4Step; 
class G4Track; 

namespace ldmx {

    /**
     * @class TargetProcessFilter
     * @brief Biases Geant4 to only process events where PN reaction occurred in the target
     */
    class TargetProcessFilter : public UserAction {

        public:

            /**
             * Class constructor.
             */
            TargetProcessFilter(const std::string& name, Parameters& parameters);

            /// Destructor
            ~TargetProcessFilter();

            /**
             * Implementmthe stepping action which performs the target volume biasing.
             * @param step The Geant4 step.
             */
            void stepping(const G4Step* step) final override;

            /**
             * End of event action.
             */
            void EndOfEventAction(const G4Event*) final override;

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
                return { TYPE::EVENT, TYPE::STACKING, TYPE::STEPPING }; 
            } 

        private:

            /** Pointer to the current track being processed. */
            G4Track* currentTrack_{nullptr};

            /** Flag indicating if the reaction of intereset occurred. */
            bool reactionOccurred_{false};

            /// The process to bias
            std::string process_{""}; 

    }; // ldmx

} // TargetProcessFilter

#endif // BIASING_TARGETPROCESSFILTER_H
