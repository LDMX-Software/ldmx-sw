/*
 * TrackFilterPlugin.h
 * @brief
 * @author JeremyMcCormick, SLAC
 */

#ifndef SIMPLUGINS_TRACKFILTERPLUGIN_H_
#define SIMPLUGINS_TRACKFILTERPLUGIN_H_

// LDMX
#include "SimCore/UserTrackInformation.h"
#include "SimPlugins/UserActionPlugin.h"
#include "SimCore/TrackFilter.h"
#include "SimPlugins/TrackFilterMessenger.h"

namespace ldmx {

    class TrackFilterPlugin : public UserActionPlugin {

        public:

            /**
             * Class constructor.
             */
            TrackFilterPlugin() {
                messenger_ = new TrackFilterMessenger(this);
            }

            /**
             * Class destructor.
             */
            ~TrackFilterPlugin() {

                for (auto filterChain : filterChains_) {
                    delete filterChain;
                }
                filterChains_.clear();

                delete messenger_;
            }

            /**
             * Get the name of the plugin.
             * @return The name of the plugin.
             */
            virtual std::string getName() {
                return "TrackFilterPlugin";
            }

            /**
             * Return whether this plugin defines a tracking action (true for this plugin).
             * @return Return whether this plugin defines a tracking action.
             */
            bool hasTrackingAction() {
                return true;
            }

            void postTracking(const G4Track* aTrack) {
                bool save = TrackFilterChain::process(aTrack, filterChains_);
                auto trackInfo = dynamic_cast<UserTrackInformation*>(aTrack->GetUserInformation());
                if (save) {
                    trackInfo->setSaveFlag(true);
                    if (verbose_ > 1) {
                        std::cout << "[ TrackFilterPlugin ] : set save flag for track ID " << aTrack->GetTrackID()
                                << ", KE = " << (aTrack->GetVertexKineticEnergy())
                                << ", PDGID = " << aTrack->GetDynamicParticle()->GetPDGcode();
                        if (aTrack->GetCreatorProcess()) {
                            std::cout << ", process = " << aTrack->GetCreatorProcess()->GetProcessName();
                        }
                        std::cout << std::endl;
                    }
                }
            }

            void addFilterChain(TrackFilterChain* filterChain) {
                filterChains_.push_back(filterChain);
            }

        private:

            TrackFilterMessenger* messenger_;

            std::vector<TrackFilterChain*> filterChains_;
    };
}

#endif /* SIMPLUGINS_TRACKFILTERPLUGIN_H_ */
