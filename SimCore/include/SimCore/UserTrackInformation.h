/**
 * @file UserTrackInformation.h
 * @brief Class providing extra information associated to a Geant4 track
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */
#ifndef SIMCORE_USERTRACKINFORMATION_H_
#define SIMCORE_USERTRACKINFORMATION_H_

#include "G4VUserTrackInformation.hh"
#include "G4ThreeVector.hh"

namespace ldmx {

    /**
     * @class UserTrackInformation
     * @note Provides extra information associated to a Geant4 track
     */
    class UserTrackInformation : public G4VUserTrackInformation {

        public:

            UserTrackInformation() {
            }

            virtual ~UserTrackInformation() {
            }

            /**
             * Overload pure virtual method (we don't implement it!).
             */
            virtual void Print() const {;}

            /**
             * Get the flag which indicates whether this track should be saved
             * as a Trajectory.
             * @return The save flag.
             */
            bool getSaveFlag() { return saveFlag_; }

            /**
             * Set the save flag so the associated track will be persisted
             * as a Trajectory.
             * @param saveFlag True to save the associated track.
             */
            void setSaveFlag(bool saveFlag) { saveFlag_ = saveFlag; }

            /**
             * Get the initial momentum 3-vector of the track [MeV].
             * @return The initial momentum of the track.
             */
            const G4ThreeVector& getInitialMomentum() { 
                return initialMomentum_; 
            } 
           
            /**
             * Set the initial momentum of the associated track.
             * @param p The initial momentum of the track.
             */ 
            void setInitialMomentum(const G4ThreeVector& p) { 
                initialMomentum_.set(p.x(), p.y(), p.z()); 
            } 
        
        private:

            /** Flag for saving the track as a Trajectory. */
            bool saveFlag_{false};

            /** The initial momentum of the track. */
            G4ThreeVector initialMomentum_;
    };
}

#endif
