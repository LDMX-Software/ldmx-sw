/**
 * @file EcalProcessFilter.h
 * @brief User action plugin that biases Geant4 to only process events which
 *        involve a photonuclear reaction in the ECal.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef BIASING_ECALPROCESSFILTER_H
#define BIASING_ECALPROCESSFILTER_H

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
#include "Biasing/TargetBremFilter.h"
#include "SimCore/UserTrackInformation.h"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimApplication/UserAction.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Parameters.h" 

namespace ldmx {

    class EcalProcessFilter : public UserAction {

        public:

            /**
             *
             */
            EcalProcessFilter(const std::string& name, Parameters& parameters);

            /// Destructor
            ~EcalProcessFilter();

            void stepping(const G4Step* step) final override;

            void PostUserTrackingAction(const G4Track*) final override; 

            /**
             * Classify a new track which postpones track processing.
             * Track processing resumes normally if a target PN interaction occurred.
             * @param aTrack The Geant4 track.
             * @param currentTrackClass The current track classification.
             */
            G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* aTrack, 
                    const G4ClassificationOfNewTrack& currentTrackClass) final override;

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

            /// Retrieve the type of actions this class defines
            std::vector< TYPE > getTypes() final override { 
                return { TYPE::TRACKING, TYPE::STACKING, TYPE::STEPPING }; 
            } 

        private:

            /** Pointer to the current track being processed. */
            G4Track* currentTrack_{nullptr};

            /** List of volumes to apply filter to. */
            std::vector<std::string> volumes_; 

            /** List of volumes to bound the particle to. */
            std::vector<std::string> boundVolumes_;

            /// Process to filter
            std::string process_{""}; 

            /** PN gamma parent ID. */
            double photonGammaID_{-1}; 

    }; // EcalProcessFilter 
}

#endif // BIASING_ECALPROCESSFILTER_H
