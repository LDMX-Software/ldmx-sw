#ifndef TRACKING_EVENT_REDUCEDTRACK_H_
#define TRACKING_EVENT_REDUCEDTRACK_H_

//----------------------//
//   C++ Standard Lib   //
//----------------------//
#include <iostream>
#include <optional>
#include <vector>
#include <array>

//----------//
//   ROOT   //
//----------//
#include "TObject.h"

// --- ACTS --- //
//#include "Acts/Definitions/TrackParametrization.hpp"
//#include "Acts/EventData/TrackParameters.hpp"

namespace ldmx {

    class ReducedTrack {
    public:
        
        ReducedTrack() = default;
        
        /**
         * Destructor.
         *
         * Currently, the destructor does nothing.
         */
        virtual ~ReducedTrack() = default;
        
        /**
         * Print the string representation of this object.
         *
         * This class is needed by ROOT when building the dictionary.
         */
        void Print() const;
        
        // To match the Framework Bus clear. It's doing nothing
        void Clear(){};
        
        void setNhits(int nhits) { n_hits_ = nhits; }
        int getNhits() const { return n_hits_; }
        
        void setNdf(int ndf) { ndf_ = ndf; }
        int getNdf() const { return ndf_; };
        
        void setNsharedHits(int nsh) { n_shared_hits_ = nsh; }
        int getNsharedHits() const { return n_shared_hits_; }
        
        void setChi2(double chi2) { chi2_ = chi2; }
        double getChi2() const { return chi2_; }
        
        //  !! NEEDS TO BE CONFIGURED !!
        //    void setTrackID(int trackid) { trackID_ = trackid; };
        //    int getTrackID() const { return trackID_; };
        
        void setAX(double ax) { ax_ = ax; }
        double getAX() const { return ax_; }
        
        void setBX(double bx) { bx_ = bx; }
        double getBX() const { return bx_; }
        
        void setAY(double ay) { ay_ = ay; }
        double getAY() const { return ay_; }
        
        void setBY(double by) { by_ = by; }
        double getBY() const { return by_; }
        
        void setDistancetoEcalRecHit(double distance) {distance_to_Ecal_ = distance;}
        double getDistanceToRecHit() const { return distance_to_Ecal_; }
        
        void setFirstSensorPosition(const std::array<double, 3>& firstSensor) {
            firstSensor_ = firstSensor;
        }
        std::array<double, 3> getFirstSensorPosition() const { return firstSensor_; };
        
        void setSecondSensorPosition(const std::array<double, 3>& secondSensor) {
            secondSensor_ = secondSensor;
        }
        std::array<double, 3> getSecondSensorPosition() const { return secondSensor_; };
        
        void setFirstLayerEcalRecHit(const std::array<double, 3>& ecalRecHit) {
            ecalRecHit_ = ecalRecHit;
        }
        std::array<double, 3> getFirstLayerEcalRecHit() const { return ecalRecHit_; };
        
        
        void setTargetLocation(const std::array<double, 3>& target_loc) {
            targetPos_ = target_loc;
        }
        void setEcalLayer1Location(const std::array<double, 3>& ecal_loc) {
            ecalLayer1Pos_ = ecal_loc;
        }
        
        void setTargetLocation(const double& z, const double& x, const double& y) {
            targetPos_[0] = z;
            targetPos_[1] = x;
            targetPos_[2] = y;
        }
        
        void setEcalLayer1Location(const double& z, const double& x, const double& y) {
            ecalLayer1Pos_[0] = z;
            ecalLayer1Pos_[1] = x;
            ecalLayer1Pos_[2] = y;
        }
        
        std::array<double, 3> getTargetLocation() const { return targetPos_; };
        double getTargetZ() const { return targetPos_[0]; };
        double getTargetX() const { return targetPos_[1]; };
        double getTargetY() const { return targetPos_[2]; };
        
        std::array<double, 3> getEcalLayer1Location() const { return ecalLayer1Pos_; };
        double getEcalLayer1Z() const { return ecalLayer1Pos_[0]; };
        double getEcalLayer1X() const { return ecalLayer1Pos_[1]; };
        double getEcalLayer1Y() const { return ecalLayer1Pos_[2]; };
        
        // getters -- TODO use an enum instead
        
        //    double getPhi() const { return perigee_pars_[2]; };
        //    double getTheta() const { return perigee_pars_[3]; };
        
    protected:
        //Actual Track Parameters
        double ax_;
        double ay_;
        double bx_;
        double by_;
        double distance_to_Ecal_;
        
        std::array<double, 3> firstSensor_;
        std::array<double, 3> secondSensor_;
        std::array<double, 3> ecalRecHit_;
        
        int n_hits_;
        int ndf_;
        int n_shared_hits_;
        double chi2_;
        
        // The target location
        std::array<double, 3> targetPos_;
        // The ecal first layer position
        std::array<double, 3> ecalLayer1Pos_;
        
        // ID of the matched particle in the SimParticles map
        //    int trackID_{-1}; COME BACK AND SETUP TRACK ID ONCE YOU CONFIGURE TRUTH STUFF
        
        /// Class declaration needed by the ROOT dictionary.
        ClassDef(ReducedTrack, 1);
        
    };  // Track

    typedef std::vector<ldmx::ReducedTrack> ReducedTracks;

}  // namespace ldmx

#endif // TRACKING_EVENT_REDUCEDTRACK_H_
