/**
 * @file TargetProcessFilter.h
 * @brief Class defining a UserActionPlugin that biases Geant4 to only process events which
 *        involve a photonuclear reaction in the target
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef BIASING_TARGETPROCESSFILTER_H
#define BIASING_TARGETPROCESSFILTER_H

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimApplication/UserAction.h"

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

            /** The volume name of the LDMX target. */
            std::string volumeName_{"target_PV"};

            /** Brem photon energy threshold. */
            double photonEnergyThreshold_{2500}; // MeV

            /** Flag indicating if the reaction of intereset occurred. */
            bool reactionOccurred_{false};

            /// The process to bias
            std::string process_{""}; 

    }; // ldmx

} // TargetProcessFilter

#endif // BIASING_TARGETPROCESSFILTER_H
