/*
 * TrackFilterPlugin.h
 * @brief
 * @author JeremyMcCormick, SLAC
 */

#ifndef SIMPLUGINS_TRACKFILTERPLUGIN_H_
#define SIMPLUGINS_TRACKFILTERPLUGIN_H_

#include "SimCore/UserTrackInformation.h"
#include "SimPlugins/UserActionPlugin.h"
#include "SimCore/TrackFilter.h"
#include "SimPlugins/TrackFilterMessenger.h"

#include <iostream>

namespace ldmx {

    /**
     * @class TrackFilterPlugin
     * @brief Sim plugin for creating track filters to flag tracks for saving
     * @note Full documentation is provided by the
     * <a href="https://github.com/LDMXAnalysis/ldmx-sw/wiki/Track-Filtering-Documentation">Track Filtering Documentation</a>
     */
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

                for (auto entry : filterChains_) {
                    delete entry.second;
                }
                filterChains_.clear();

                postFilterChains_.clear();

                preFilterChains_.clear();

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
                process(postFilterChains_, aTrack);
            }

            void preTracking(const G4Track* aTrack) {
                process(preFilterChains_, aTrack);
            }

            void addPostFilterChain(TrackFilterChain* filterChain) {
                postFilterChains_.push_back(filterChain);
                filterChains_[filterChain->getName()] = filterChain;
            }

            void addPreFilterChain(TrackFilterChain* filterChain) {
                preFilterChains_.push_back(filterChain);
                filterChains_[filterChain->getName()] = filterChain;
            }

            TrackFilterChain* getFilterChain(std::string name) {
                return filterChains_[name];
            }

            void process(std::vector<TrackFilterChain*> filterChains, const G4Track* aTrack) {
                bool save = TrackFilterChain::process(aTrack, filterChains);
                auto trackInfo = dynamic_cast<UserTrackInformation*>(aTrack->GetUserInformation());
                if (save) {
                    trackInfo->setSaveFlag(true);
                    if (verbose_ > 1) {
                        std::cout << "[ TrackFilterPlugin ] : set save flag for track " << aTrack->GetTrackID()
                                << ", KE = " << (aTrack->GetVertexKineticEnergy())
                                << ", PDGID = " << aTrack->GetDynamicParticle()->GetPDGcode();
                        if (aTrack->GetCreatorProcess()) {
                            std::cout << ", process = " << aTrack->GetCreatorProcess()->GetProcessName();
                        }
                        std::cout << std::endl;
                    }
                }
            }

            std::ostream& print(std::ostream& os, TrackFilterChain* filterChain) {
                os << filterChain->getName() << "[";
                if (isPreFilterChain(filterChain)) {
                    os << "pre";
                } else {
                    os << "post";
                }
                os << "]" << std::endl;
                for (auto filter : filterChain->getFilters()) {
                    os << "  ";
                    filter->print(os);
                    os << std::endl;
                }
                return os;
            }

            std::ostream& print(std::ostream& os) {
                for (auto entry : filterChains_) {
                    print(os, entry.second);
                }
                return os;
            }

        private:

            bool isPostFilterChain(TrackFilterChain* filterChain) {
                return find(postFilterChains_.begin(), postFilterChains_.end(), filterChain) != postFilterChains_.end();
            }

            bool isPreFilterChain(TrackFilterChain* filterChain) {
                return find(preFilterChains_.begin(), preFilterChains_.end(), filterChain) != preFilterChains_.end();
            }

        private:

            TrackFilterMessenger* messenger_;

            // list of post-tracking filter chains
            std::vector<TrackFilterChain*> postFilterChains_;

            // list of pre-tracking filter chains
            std::vector<TrackFilterChain*> preFilterChains_;

            std::map<std::string, TrackFilterChain*> filterChains_;
    };
}

#endif /* SIMPLUGINS_TRACKFILTERPLUGIN_H_ */
