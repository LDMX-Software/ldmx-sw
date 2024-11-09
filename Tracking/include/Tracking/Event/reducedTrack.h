#ifndef TRACKING_EVENT_TRACK_H_
#define TRACKING_EVENT_TRACK_H_

//----------------------//
//   C++ Standard Lib   //
//----------------------//
#include <iostream>
#include <optional>
#include <vector>

//----------//
//   ROOT   //
//----------//
#include "TObject.h"

// --- ACTS --- //
//#include "Acts/Definitions/TrackParametrization.hpp"
//#include "Acts/EventData/TrackParameters.hpp"

namespace ldmx {

/// This enum describes the type of TrackState
/// RefPoint is wrt to a line parallel to the Z axis located at the refPoint
/// stored in the TrackState AtTarget is wrt the target surface: i.e. a surface
/// at the refPoint with orientation as the ACTS Tracking Frame
/// AtFirstMeasurement: track state at the first measurment on track.
/// For the recoil "first" means closest to the target, for the tagger it means
/// farthest from the target

/// AtLastMeasurement : track state at the last measurement on track.
/// For the recoil it means closest to the ECAL, for the tagger closest to the
/// target.

/**
 * Implementation of a track object.
 *
 * This class encapsulates all the information of a particle trajectory in the
 * tracker
 *
 */

class reducedTrack {
public:
    
    Track(){};
    
    /**
     * Destructor.
     *
     * Currently, the destructor does nothing.
     */
    virtual ~Track(){};
    
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
    
    void setFirstSensorPosition(const std::vector<double>& firstSensor) {
        firstSensor_ = firstSensor;
    }
    std::vector<double> getFirstSensorPosition() const { return firstSensor_; };

    void setSecondSensorPosition(const std::vector<double>& secondSensor) {
        secondSensor_ = secondSensor;
    }
    std::vector<double> getSecondSensorPosition() const { return secondSensor_; };

    void setFirstLayerEcalRecHit(const std::vector<double>& ecalRecHit) {
        ecalRecHit_ = ecalRecHit;
    }
    std::vector<double> getFirstLayerEcalRecHit() const { return ecalRecHit_; };

    
    void setTargetLocation(const std::vector<double>& target_loc) {
        targetPos_ = target_loc;
    }
    void setEcalLayer1Location(const std::vector<double>& ecal_loc) {
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
    
    std::vector<double> getTargetLocation() const { return targetPos_; };
    double getTargetZ() const { return targetPos_[0]; };
    double getTargetX() const { return targetPos_[1]; };
    double getTargetY() const { return targetPos_[2]; };
    
    std::vector<double> getEcalLayer1Location() const { return ecalLayer1Pos_; };
    double getEcalLayer1Z() const { return ecalLayer1Pos_[0]; };
    double getEcalLayer1X() const { return ecalLayer1Pos_[1]; };
    double getEcalLayer1Y() const { return ecalLayer1Pos_[2]; };
    
    // getters -- TODO use an enum instead
    
//    double getPhi() const { return perigee_pars_[2]; };
//    double getTheta() const { return perigee_pars_[3]; };
    
protected:
    //Actual Track Parameters
    double ax_{0};
    double ay_{0};
    double bx_{0};
    double by_{0};
    double distance_to_Ecal_{0};
    
    std::vector<double> firstSensor_{0., 0., 0.};
    std::vector<double> secondSensor_{0., 0., 0.};
    std::vector<double> ecalRecHit_{0., 0., 0.};

    int n_hits_{0};
    int ndf_{0};
    int n_shared_hits_{0};
    double chi2_{0};
    
    // The target location
    std::vector<double> targetPos_{0., 0., 0.};
    // The ecal first layer position
    std::vector<double> ecalLayer1Pos_{0., 0., 0.};
    
    // ID of the matched particle in the SimParticles map
//    int trackID_{-1}; COME BACK AND SETUP TRACK ID ONCE YOU CONFIGURE TRUTH STUFF
    
    /// Class declaration needed by the ROOT dictionary.
    ClassDef(reducedTrack, 2);
    
};  // Track

typedef std::vector<ldmx::reducedTrack> reducedTracks;

}  // namespace ldmx

#endif  // TRACKING_EVENT_TRACK_H_
