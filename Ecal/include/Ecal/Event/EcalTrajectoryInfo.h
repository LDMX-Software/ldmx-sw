/**
 * @file EcalTrajectoryInfo.h
 * @brief Class that determines MIP tracking information using ECAL hit
 * information
 * @author Jihoon Yoo, Tamas Vami (UCSB)
 */

#ifndef EVENT_ECALMIPCOLLECTION_H_
#define EVENT_ECALMIPCOLLECTION_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>
#include <map>
#include <utility>

//----------//
//   ROOT   //
//----------//
#include <Math/Vector3D.h>

#include "TObject.h"

namespace ldmx {

typedef std::pair<float, float> XYCoords;

// MIP tracking:  Class for storing hit information for tracking in a
// convenient way
struct HitData {
  int layer_;
  ROOT::Math::XYZVector pos_;
};

class EcalTrajectoryInfo {
 public:
  /** Constructor */
  EcalTrajectoryInfo() = default;

  /** Destructor */
  virtual ~EcalTrajectoryInfo() = default;

  /**
   * Print the string representation of this object.
   */
  friend std::ostream& operator<<(std::ostream& o, const EcalTrajectoryInfo& d);

  void clear();

  const std::vector<XYCoords>& getEleTrajectory() const {
    return ele_trajectory_;
  }
  void setEleTrajectory(const std::vector<XYCoords>& ele_trajectory) {
    ele_trajectory_ = ele_trajectory;
  }

  const std::vector<XYCoords>& getPhotonTrajectory() const {
    return photon_trajectory_;
  }
  void setPhotonTrajectory(const std::vector<XYCoords>& photon_trajectory) {
    photon_trajectory_ = photon_trajectory;
  }

  const std::vector<HitData>& getTrackingHitList() const {
    return tracking_hit_list_;
  }
  void setTrackingHitList(const std::vector<HitData>& hits) {
    tracking_hit_list_ = hits;
  }

 private:
  std::vector<XYCoords> ele_trajectory_;
  std::vector<XYCoords> photon_trajectory_;
  std::vector<HitData> tracking_hit_list_;

  ClassDef(EcalTrajectoryInfo, 2);
};  // EcalTrajectoryInfo

}  // namespace ldmx

#endif  // EVENT_ECALMIPCOLLECTION_H_
