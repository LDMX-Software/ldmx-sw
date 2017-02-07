/**
 * @file TargetBremFilter.h
 * @class TargetBremFilter
 * @brief Class defining a UserActionPlugin that allows a user to filter out 
 *        events that don't result in a brem within the target.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMPLUGINS_TARGETBREMFILTER_H_
#define SIMPLUGINS_TARGETBREMFILTER_H_

// Geant4
#include "G4RunManager.hh"

// LDMX
#include "SimPlugins/UserActionPlugin.h"
#include "Biasing/BiasingMessenger.h"

namespace ldmx {

    class TargetBremFilter : public UserActionPlugin {

        public:

            /**
             * Class constructor.
             */
            TargetBremFilter();

            /**
             * Class destructor.
             */
            ~TargetBremFilter();

            /**
             * Get the name of the plugin.
             * @return The name of the plugin.
             */
            virtual std::string getName() {
                return "TargetBremFilter";
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
             * Implement the stepping action which performs the target volume biasing.
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

            /** The volume name of the LDMX target. */
            G4String volumeName_{"target_PV"};

            /** Brem photon energy threshold. */
            double photonEnergyThreshold_{2500}; // MeV

    }; // TargetBremFilter
}

#endif // SIMPLUGINS_TARGETBREMFILTER_H__
