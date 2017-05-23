/**
 * @file TargetBremFilter.h
 * @class TargetBremFilter
 * @brief Class defining a UserActionPlugin that allows a user to filter out 
 *        events that don't result in a brem within the target.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef BIASING_TARGETBREMFILTER_H_
#define BIASING_TARGETBREMFILTER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <algorithm>

// Geant4
#include "G4RunManager.hh"

// LDMX
#include "SimPlugins/UserActionPlugin.h"
#include "Biasing/BiasingMessenger.h"
#include "Biasing/TargetBremFilterMessenger.h"

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
             * Get whether this plugin implements the event action.
             * @return True if the plugin implements the event action.
             */
            virtual bool hasEventAction() { 
                return true;
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
             * End of event action.
             */
            virtual void endEvent(const G4Event*);

            /**
             * Classify a new track which postpones track processing.
             * Track processing resumes normally if a target PN interaction occurred.
             * @param aTrack The Geant4 track.
             * @param currentTrackClass The current track classification.
             */
            G4ClassificationOfNewTrack stackingClassifyNewTrack(const G4Track* aTrack, const G4ClassificationOfNewTrack& currentTrackClass);

            /**
             *
             */
            static std::vector<G4Track*> getBremGammaList() { return bremGammaTracks_; };

            /** 
             * Enable/disable killing of the recoil electron track.  If the 
             * recoil track is killed, only the brem gamma is propagated.
             */
            void setKillRecoilElectron(bool killRecoilElectron) { killRecoilElectron_ = killRecoilElectron; };

            /**
             *
             */
            static void removeBremFromList(G4Track* track);

        private:
            
            /** Messenger used to pass arguments to this class. */
            //TargetBremFilterMessenger* messenger_{new TargetBremFilterMessenger{this}};

            static std::vector<G4Track*> bremGammaTracks_; 

            /** The volume name of the LDMX target. */
            G4String volumeName_{"target_PV"};

            /** Recoil electron threshold. */
            double recoilElectronThreshold_{1500}; // MeV

            /** Flag indicating if the recoil electron track should be killed. */
            bool killRecoilElectron_{false};


    }; // TargetBremFilter
}

#endif // BIASING_TARGETBREMFILTER_H__
