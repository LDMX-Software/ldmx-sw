/**
 * @file TargetDarkBremFilter.h
 * @class TargetDarkBremFilter
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

//------------//
//   Geant4   //
//------------//
#include "G4RunManager.hh"

//----------//
//   LDMX   //
//----------//
#include "SimPlugins/UserActionPlugin.h"
#include "Biasing/BiasingMessenger.h"
#include "Biasing/TargetDarkBremFilterMessenger.h"

namespace ldmx {

    class TargetDarkBremFilterMessenger; 

    class TargetDarkBremFilter : public UserActionPlugin {

        public:

            /**
             * Class constructor.
             */
            TargetDarkBremFilter();

            /**
             * Class destructor.
             */
            ~TargetDarkBremFilter();

            /**
             * Get the name of the plugin.
             * @return The name of the plugin.
             */
            virtual std::string getName() {
                return "TargetDarkBremFilter";
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
            G4ClassificationOfNewTrack stackingClassifyNewTrack(const G4Track* aTrack, 
                    const G4ClassificationOfNewTrack& currentTrackClass);

            /**
             *
             */
            static std::vector<G4Track*> getBremGammaList() { return bremGammaTracks_; }

            /** 
             * Enable/disable killing of the recoil electron track.  If the 
             * recoil track is killed, only the brem gamma is propagated.
             */
            void setKillRecoilElectron(bool killRecoilElectron) { 
                killRecoilElectron_ = killRecoilElectron; 
            }

            /** 
             * @param volume Set the volume that the filter will be applied to. 
             */
            void setVolume(std::string volumeName) { volumeName_ = volumeName; }; 

            /**
             * Set the energy threshold that the recoil electron must exceed.
             */
            void setRecoilEnergyThreshold(double recoilEnergyThreshold) { 
                recoilEnergyThreshold_ = recoilEnergyThreshold; 
            }

            /**
             * Set the minimum energy that the brem gamma must have.
             */
            void setBremEnergyThreshold(double bremEnergyThreshold) { 
                bremEnergyThreshold_ = bremEnergyThreshold; 
            }

            /**
             *
             */
            static void removeBremFromList(G4Track* track);

        private:
            
            /** Messenger used to pass arguments to this class. */
            TargetDarkBremFilterMessenger* messenger_{nullptr};

            static std::vector<G4Track*> bremGammaTracks_; 

            /** The volume that the filter will be applied to. */
            G4String volumeName_{"target_PV"};

            /** Recoil electron threshold. */
            double recoilEnergyThreshold_{1500}; // MeV

            /** Brem gamma energy treshold. */
            double bremEnergyThreshold_{0}; 

            /** Flag indicating if the recoil electron track should be killed. */
            bool killRecoilElectron_{false};


    }; // TargetDarkBremFilter
}

#endif // BIASING_TARGETBREMFILTER_H__
