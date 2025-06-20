/**
 * @file EcalMipCollection.h
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
#include <TVector3.h>

#include "TObject.h"

namespace ldmx {

typedef std::pair<float, float> XYCoords;

// MIP tracking:  Class for storing hit information for tracking in a
// convenient way
struct HitData {
  int layer;
  TVector3 pos;
};

class EcalMipCollection {
 public:
  /** Constructor */
  EcalMipCollection() = default;

  /** Destructor */
  virtual ~EcalMipCollection() = default;

  /**
   * Print the string representation of this object.
   * This class is needed by ROOT when building the dictionary.
   */
  void Print() const;

  void Clear();

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

  ClassDef(EcalMipCollection, 1);
};  // EcalMipCollection

}  // namespace ldmx

#endif  // EVENT_ECALMIPCOLLECTION_H_
