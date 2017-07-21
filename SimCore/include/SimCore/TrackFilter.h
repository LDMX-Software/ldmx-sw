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
#include "G4Region.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"

#include <unordered_set>
#include <vector>
#include <iostream>

namespace ldmx {

    /**
     * @class TrackFilter
     * @brief Abstract class for creating user track filters
     * @note Filters that have multiple rules generally OR them together,
     * e.g. for multiple physics processes.  So only one of these sub-rules
     * must pass for the filter to return true.
     */
    class TrackFilter {

        public:

            TrackFilter() {
            }

            virtual ~TrackFilter() {
            }

            /**
             * Return whether this filter passes or not for the given track.
             * @return True if filter passes for this track.
             */
            virtual bool passes(const G4Track*) = 0;

            /**
             * Utility method to process a track with a list of filters.
             * All filters must return true for this to pass.
             * @return True if all filters pass.
             */
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

            /**
             * Print the filter rules to the output stream.
             * @param os The output stream.
             * @return The same output stream.
             */
            virtual std::ostream& print(std::ostream& os) {
                return os;
            }
    };

    /**
     * @class TrackFilterChain
     * @brief Implements a chain of one or more track filters that all must pass
     */
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

            /**
             * Add a filter to the chain.
             * @param filter The filter to add.
             */
            void addFilter(TrackFilter* filter) {
                filters_.push_back(filter);
            }

            /**
             * Set the name of this filter chain.
             * @param name The name of this filter chain.
             */
            void setName(std::string name) {
                name_ = name;
            }

            /**
             * Get the name of the filter chain.
             * @return The name of the filter chain.
             */
            const std::string& getName() {
                return name_;
            }

            const std::vector<TrackFilter*> getFilters() {
                return filters_;
            }

        private:

            std::string name_;
            std::vector<TrackFilter*> filters_;
    };

    /**
     * @class TrackPassFilter
     * @brief A track filter that always passes
     */
    class TrackPassFilter : public TrackFilter {

        public:

            virtual bool passes(const G4Track*) {
                return true;
            }

            virtual std::ostream& print(std::ostream& os) {
                os << "pass";
                return os;
            }
    };

    /**
     * @class TrackEnergyFilter
     * @brief Filters tracks on kinetic energy threshold
     */
    class TrackEnergyFilter : public TrackFilter {

        public:

            TrackEnergyFilter(double threshold) : threshold_(threshold) {
            }

            bool passes(const G4Track* aTrack) {
                return (aTrack->GetVertexKineticEnergy()) > threshold_;
            }

            void setThreshold(double threshold) {
                threshold_ = threshold;
            }

            std::ostream& print(std::ostream& os) {
                os << "KE > " << (threshold_ / MeV) << " MeV";
                return os;
            }

        private:

            double threshold_{0.};
    };

    /**
     * @class TrackPDGCodeFilter
     * @brief Filters tracks based on PDG code of the particle
     */
    class TrackPDGCodeFilter : public TrackFilter {

        public:

            TrackPDGCodeFilter() {
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

            void addParticle(std::string particleName) {
                G4ParticleDefinition* particleDef = G4ParticleTable::GetParticleTable()->FindParticle(particleName);
                if (particleDef) {
                    pdgids_.insert(particleDef->GetPDGEncoding());
                } else {
                    G4Exception("", "", FatalException, std::string("Unknown particle name: " + particleName).c_str());
                }
            }

            std::ostream& print(std::ostream& os) {
                os << "PDG Codes:";
                for (int pdgid : pdgids_) {
                    os << " " << pdgid;
                }
                return os;
            }


        private:

            std::unordered_set<int> pdgids_;
    };

    /**
     * @class TrackProcessFilter
     * @brief Filters tracks based on creator physics process type
     */
    class TrackProcessFilter : public TrackFilter {

        public:

            TrackProcessFilter() {
            }

            virtual bool passes(const G4Track* aTrack) {
                const G4VProcess* process = aTrack->GetCreatorProcess();
                if (process) {
                    // during comparison, ignore any "biasWrapper()" extra text
                    std::string currentProcessName = process->GetProcessName();
                    if (currentProcessName.find("biasWrapper") != std::string::npos) { 
                        std::size_t pos = currentProcessName.find_first_of("(") + 1;
                        currentProcessName = currentProcessName.substr(pos, currentProcessName.size() - pos - 1); 
                    }  
                    
                    for (const auto& processName : processNames_) {
                        if (!currentProcessName.compare(processName)) return true;
                    }
                }
                return false;
            }

            void addProcess(std::string processName, bool exactMatch) {
                processNames_.push_back(processName);
            }

            std::ostream& print(std::ostream& os) {
                os << "processes:";
                for (auto processName : processNames_) {
                    os << " " << processName;
                }
                return os;
            }

        protected:

            /** Names of physics processes to save. */
            std::vector<std::string> processNames_;

            /** Map indicating whether process name needs to be matched exactly. */
            std::map<std::string, bool> exactMatch_;

    };

    /**
     * @clsas TrackParentProcessFilter
     * @brief Filters tracks based on matching process type of secondaries
     */
    class TrackParentProcessFilter : public TrackProcessFilter {

        public:

            TrackParentProcessFilter() {
            }

            bool passes(const G4Track* aTrack) {
                static G4TrackingManager* mgr = G4RunManagerKernel::GetRunManagerKernel()->GetTrackingManager();
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

            std::ostream& print(std::ostream& os) {
                os << "daughter processes:";
                for (auto processName : processNames_) {
                    os << " " << processName;
                }
                return os;
            }
    };

    /**
     * @class TrackRegionFilter
     * @brief Filters tracks based on name of detector region
     */
    class TrackRegionFilter : public TrackFilter {

        public:

            TrackRegionFilter() {
            }

            void addRegion(std::string regionName, bool regionSave) {
                regions_.insert(regionName);
                regionSave_[regionName] = regionSave;
            }

            bool passes(const G4Track* aTrack) {
                G4Region* region = aTrack->GetLogicalVolumeAtVertex()->GetRegion();
                if (regions_.find(region->GetName()) != regions_.end()) {
                    return regionSave_[region->GetName()];
                } else {
                    return false;
                }
            }

            std::ostream& print(std::ostream& os) {
                os << "regions:";
                for (auto region : regions_) {
                    os << " " << region;
                }
                return os;
            }

        private:

            std::unordered_set<std::string> regions_;
            std::map<std::string, bool> regionSave_;
    };

    /**
     * @class TrackVolumeFilter
     * @brief Filters tracks based on name of logical volume
     */
    class TrackVolumeFilter : public TrackFilter {

        public:

            TrackVolumeFilter() {
            }

            void addVolume(std::string volumeName, bool volumeSave) {
                volumes_.insert(volumeName);
                volumeSave_[volumeName] = volumeSave;
            }

            bool passes(const G4Track* aTrack) {
                const G4LogicalVolume* vol = aTrack->GetLogicalVolumeAtVertex();
                if (volumes_.find(vol->GetName()) != volumes_.end()) {
                    return volumeSave_[vol->GetName()];
                } else {
                    return false;
                }
            }

            std::ostream& print(std::ostream& os) {
                os << "volumes:";
                for (auto volume : volumes_) {
                    os << " " << volume;
                }
                return os;
            }

        private:

            std::unordered_set<std::string> volumes_;
            std::map<std::string, bool> volumeSave_;
    };

    /**
     * @class TrackIDFilter
     * @brief Filters tracks based on track ID
     */
    class TrackIDFilter : public TrackFilter {

        public:

            TrackIDFilter() {
            }

            void addTrackID(int trackID) {
                trackIDs_.insert(trackID);
            }

            bool passes(const G4Track* aTrack) {
                return trackIDs_.find(aTrack->GetTrackID()) != trackIDs_.end();
            }

        private:

            std::unordered_set<int> trackIDs_;
    };
}

#endif /* SIMCORE_TRACKFILTER_H_ */
