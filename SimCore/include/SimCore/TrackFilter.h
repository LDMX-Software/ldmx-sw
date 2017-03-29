/*
 * @file TrackFilter.h
 * @brief Utility classes for filtering tracks
 * @author JeremyMcCormick, SLAC
 */

#ifndef SIMCORE_TRACKFILTER_H_
#define SIMCORE_TRACKFILTER_H_

#include "G4VProcess.hh"
#include "G4TrackingManager.hh"
#include "G4RunManagerKernel.hh"

#include <unordered_set>
#include <vector>

#include <typeinfo>

namespace ldmx {

    class TrackFilter {

        public:

            TrackFilter(std::string name) : name_(name) {;}

            virtual ~TrackFilter() {;}

            virtual bool passes(const G4Track*) = 0;

            const std::string& getName() {
                return name_;
            }

            static bool filter(const std::vector<TrackFilter*> filters, const G4Track* aTrack) {
                bool save = false;
                for (auto filter : filters) {
                    save = filter->passes(aTrack);
                    if (!save) {
                        break;
                    }
                }
                return save;
            }

        protected:

            std::string name_;
    };

    class TrackFilterChain {

        public:

            TrackFilterChain(std::string name) : name_(name) {
            }

            virtual ~TrackFilterChain() {
                for (auto filter : filters_) {
                    delete filter;
                }
                filters_.clear();
            }

            /**
             * Process the track using all filters in the chain.
             * All filters must pass for the chain to return true.
             * @param aTrack The Geant4 track to process.
             * @return True if all filters pass.
             */
            bool process(const G4Track* aTrack) {
                return TrackFilter::filter(filters_, aTrack);
            }

            /**
             * Process a track using multiple filter chains.
             * Only one chain needs to pass for this to return true.
             * @param aTrack The Geant4 track to process.
             * @return True if at least one chain passes.
             */
            static bool process(const G4Track* aTrack, const std::vector<TrackFilterChain*>& filters) {
                bool save = false;
                for (auto filterChain : filters) {
                    save = filterChain->process(aTrack);
                    if (save) {
                        break;
                    }
                }
                return save;
            }

            void addFilter(TrackFilter* filter) {
                filters_.push_back(filter);
            }

            void setName(std::string name) {
                name_ = name;
            }

            const std::string& getName() {
                return name_;
            }

        private:

            std::string name_;
            std::vector<TrackFilter*> filters_;
    };

    class TrackEnergyFilter : public TrackFilter {

        public:

            TrackEnergyFilter(double threshold) : TrackFilter("Vertex KE Threshold"), threshold_(threshold) {
            }

            bool passes(const G4Track* aTrack) {
                return (aTrack->GetVertexKineticEnergy()) > threshold_;
            }

            void setThreshold(double threshold) {
                threshold_ = threshold;
            }

        private:

            double threshold_{0.};
    };

    class TrackPDGCodeFilter : public TrackFilter {

        public:

            TrackPDGCodeFilter() : TrackFilter("PDG ID") {
            }

            bool passes(const G4Track* aTrack) {
                if (pdgids_.size()) {
                    return pdgids_.find(aTrack->GetDynamicParticle()->GetPDGcode()) != pdgids_.end();
                } else {
                    return true;
                }
            }

            void addPDGCode(int pdgid) {
                pdgids_.insert(pdgid);
            }

        private:

            std::unordered_set<int> pdgids_;
    };

    class TrackProcessFilter : public TrackFilter {

        public:

            TrackProcessFilter(std::string processName, bool exactMatch) : TrackFilter("Physics Process") {
                addProcess(processName, exactMatch);
            }

            virtual bool passes(const G4Track* aTrack) {
                const G4VProcess* process = aTrack->GetCreatorProcess();
                if (process) {
                    for (const auto& processName : processNames_) {
                        if (exactMatch_[processName]) {
                            if (!processName.compare(process->GetProcessName())) {
                                return true;
                            }
                        } else {
                            if (process->GetProcessName().find(processName) != std::string::npos) {
                                return true;
                            }
                        }
                    }
                }
                return false;
            }

            void addProcess(std::string processName, bool exactMatch) {
                processNames_.push_back(processName);
                exactMatch_[processName] = exactMatch;
            }

        protected:

            /** Names of physics processes to save. */
            std::vector<std::string> processNames_;

            /** Map indicating whether process name needs to be matched exactly. */
            std::map<std::string, bool> exactMatch_;

    };

    class TrackParentProcessFilter : public TrackProcessFilter {

        public:

            TrackParentProcessFilter(std::string processName, bool exactMatch)
                : TrackProcessFilter(processName, exactMatch) {
                name_ = "Parent Physics Process";
            }

            bool passes(const G4Track* aTrack) {
                G4TrackingManager* mgr = G4RunManagerKernel::GetRunManagerKernel()->GetTrackingManager();
                if (aTrack != mgr->GetTrack()) {
                    return false;
                }
                G4TrackVector* tracks = mgr->GimmeSecondaries();
                for (auto track : *tracks) {
                    const G4VProcess* process = track->GetCreatorProcess();
                    if (process) {
                        for (const auto& processName : processNames_) {
                            if (exactMatch_[processName]) {
                                if (!processName.compare(process->GetProcessName())) {
                                    return true;
                                }
                            } else {
                                if (process->GetProcessName().find(processName) != std::string::npos) {
                                    return true;
                                }
                            }
                        }
                    }
                }
                return false;
            }
    };

    // TODO:
    // Filter for looking at the region flag where secondary saving is turned off.
    // Name of region could be parameter with true/false for saving.

}

#endif /* SIMCORE_TRACKFILTER_H_ */
