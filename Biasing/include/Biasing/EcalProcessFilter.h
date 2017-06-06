/**
 * @file EcalProcessFilter.h
 * @brief User action plugin that biases Geant4 to only process events which
 *        involve a photonuclear reaction in the ECal.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#ifndef BIASING_ECALPROCESSFILTER_H_
#define BIASING_ECALPROCESSFILTER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <algorithm>

//------------//
//   Geant4   //
//------------//
#include "G4VPhysicalVolume.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4RunManager.hh"

//----------//
//   LDMX   //
//----------//
#include "SimPlugins/UserActionPlugin.h"
#include "Biasing/BiasingMessenger.h"
#include "Biasing/EcalProcessFilterMessenger.h"
#include "Biasing/TargetBremFilter.h"
#include "SimCore/UserTrackInformation.h"

class UserTrackingAction;

namespace ldmx {

    
    // Forward declaration to avoid circular dependecies
    class EcalProcessFilterMessenger; 

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
             * Get whether this plugin implements the tracking action.
             * @return True if the plugin implements the tracking action.
             */
            bool hasTrackingAction() { 
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
             * Pre-tracking action.
             */
            void postTracking(const G4Track*); 

            /**
             * Classify a new track which postpones track processing.
             * Track processing resumes normally if a target PN interaction occurred.
             * @param aTrack The Geant4 track.
             * @param currentTrackClass The current track classification.
             */
            G4ClassificationOfNewTrack stackingClassifyNewTrack(const G4Track* aTrack, const G4ClassificationOfNewTrack& currentTrackClass);

            /** 
             * Add a volume to apply the filter to.
             *
             * @param Volume name
             */
            void addVolume(std::string volume);

            /** 
             * Add a volume to bound the particle of interest to.
             *
             * @param Volume name
             */
            void addBoundingVolume(std::string volume);

        private:

            /** Messenger used to pass arguments to this class. */
            EcalProcessFilterMessenger* messenger_{nullptr};

            /** Pointer to the current track being processed. */
            G4Track* currentTrack_{nullptr};

            /** List of volumes to apply filter to. */
            std::vector<std::string> volumes_; 

            /** List of volumes to bound the particle to. */
            std::vector<std::string> boundVolumes_;

            /** Brem photon energy threshold */
            double photonEnergyThreshold_{2500}; // MeV

            /** PN gamma parent ID. */
            double photonGammaID_{-1}; 

    }; // EcalProcessFilter 
}

#endif // BIASING_ECALPROCESSFILTER_H__
