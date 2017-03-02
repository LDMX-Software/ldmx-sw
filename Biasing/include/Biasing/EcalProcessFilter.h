/**
 * @file EcalProcessFilter.h
 * @brief User action plugin that biases Geant4 to only process events which
 *        involve a photonuclear reaction in the ECal.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#ifndef BIASINGS_ECALPROCESSFILTER_H_
#define BIASINGS_ECALPROCESSFILTER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <algorithm>

//------------//
//   Geant4   //
//------------//
#include "G4RunManager.hh"

//----------//
//   LDMX   //
//----------//
#include "SimPlugins/UserActionPlugin.h"
#include "Biasing/BiasingMessenger.h"
#include "Biasing/TargetBremFilter.h"

namespace ldmx {

    class EcalProcessFilter : public UserActionPlugin {

        public:

            /** Default Ctor */
            EcalProcessFilter();

            /** Destructor */
            ~EcalProcessFilter();

            /** @return A std::string descriptor of the class. */
            virtual std::string getName() {
                return "EcalProcessFilter";
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

            /** Brem photon energy threshold */
            double photonEnergyThreshold_{2500}; // MeV

    }; // EcalProcessFilter 
}

#endif // BIASINGS_ECALPROCESSFILTER_H__
