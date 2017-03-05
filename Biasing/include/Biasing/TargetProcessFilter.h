/**
 * @file TargetProcessFilter.h
 * @brief Class defining a UserActionPlugin that biases Geant4 to only process events which
 *        involve a photonuclear reaction in the target
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef BIASING_TARGETPROCESSFILTER_H_
#define BIASING_TARGETPROCESSFILTER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <algorithm>

// Geant4
#include "G4RunManager.hh"

// LDMX
#include "SimPlugins/UserActionPlugin.h"
#include "Biasing/BiasingMessenger.h"
#include "Biasing/TargetBremFilter.h"

namespace ldmx {

/**
 * @class TargetProcessFilter
 * @brief Biases Geant4 to only process events where PN reaction occurred in the target
 */
class TargetProcessFilter : public UserActionPlugin {

    public:

        /**
         * Class constructor.
         */
        TargetProcessFilter();

        /**
         * Class destructor.
         */
        ~TargetProcessFilter();

        /**
         * Get the name of the plugin.
         * @return The name of the plugin.
         */
        virtual std::string getName() {
            return "TargetProcessFilter";
        }

        /**
         * Get whether this plugin implements the stepping action.
         * @return True to indicate this plugin implements the stepping action.
         */
        bool hasSteppingAction() {
            return true;
        }

            /**
             * Get whether this plugin implements the stacking aciton.
             * @return True to indicate this plugin implements the stacking action.
             */
            bool hasStackingAction() { 
                return true;
            }

        /**
         * Implementmthe stepping action which performs the target volume biasing.
         * @param step The Geant4 step.
         */
        void stepping(const G4Step* step);

            /**
             * Classify a new track which postpones track processing.
             * Track processing resumes normally if a target PN interaction occurred.
             * @param aTrack The Geant4 track.
             * @param currentTrackClass The current track classification.
             */
            G4ClassificationOfNewTrack stackingClassifyNewTrack(const G4Track* aTrack, const G4ClassificationOfNewTrack& currentTrackClass);

    private:

        /** Pointer to the current track being processed. */
        G4Track* currentTrack_{nullptr};

        /** The volume name of the LDMX target. */
        G4String volumeName_{"target_PV"};

        /** Brem photon energy threshold. */
        double photonEnergyThreshold_{2500}; // MeV

};

}

#endif // BIASING_TARGETPROCESSFILTER_H__
