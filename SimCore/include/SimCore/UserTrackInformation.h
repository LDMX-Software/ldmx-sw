#ifndef SIMCORE_USERTRACKINFORMATION_H
#define SIMCORE_USERTRACKINFORMATION_H

#include "G4VUserTrackInformation.hh"
#include "G4ThreeVector.hh"

namespace ldmx {

    /**
     * Provides user defined information to associate with a Geant4 track.
     */
    class UserTrackInformation : public G4VUserTrackInformation {

        public:
            
            /// Constructor
            UserTrackInformation();

            /// Destructor
            ~UserTrackInformation(); 

            /// Print the information associated with the track. 
            void Print() const final override; 

            /**
             * Get the flag which indicates whether this track should be saved
             * as a Trajectory.
             *
             * @return The save flag.
             */
            bool getSaveFlag() { return saveFlag_; }

            /**
             * Set the save flag so the associated track will be persisted
             * as a Trajectory.
             *
             * @param[in] saveFlag True to save the associated track.
             */
            void setSaveFlag(bool saveFlag) { saveFlag_ = saveFlag; }

            /**
             * Check whether this track is a brem candidate.
             *
             * @return True if this track is a brem candidate, false otherwise. 
             */
            bool isBremCandidate() { return isBremCandidate_; }

            /**
             * Tag this flag as a brem candidate by the biasing filters. 
             *
             * @param[in] isBremCandidate flag indicating whether this track is
             *      a candidate or not. 
             */
            void tagBremCandidate(bool isBremCandidate = true) { 
                isBremCandidate_ = isBremCandidate;
            }

            /**
             * Get the initial momentum 3-vector of the track [MeV].
             *
             * @return The initial momentum of the track.
             */
            const G4ThreeVector& getInitialMomentum() { 
                return initialMomentum_; 
            } 
           
            /**
             * Set the initial momentum of the associated track.
             *
             * @param[in] p The initial momentum of the track.
             */ 
            void setInitialMomentum(const G4ThreeVector& p) { 
                initialMomentum_.set(p.x(), p.y(), p.z()); 
            } 
        
        private:

            /// Flag for saving the track as a Trajectory.
            bool saveFlag_{false};

            /// Flag indicating whether this track is a brem candidate
            bool isBremCandidate_{false}; 

            /// The initial momentum of the track.
            G4ThreeVector initialMomentum_;
    };
}

#endif
