/**
 * @file CaloCluster.h
 * @brief Class that stores calorimeter cluster information
 */

#ifndef EVENT_CALOCLUSTER_H_
#define EVENT_CALOCLUSTER_H_

// ROOT
#include "TObject.h"  //For ClassDef
#include "TString.h"

// STL
#include <iostream>
#include <set>

// ldmx-sw
#include "Recon/Event/CalorimeterHit.h"

namespace ldmx {

/**
 * @class CaloCluster
 * @brief Stores cluster information from the ECal
 */
class CaloCluster {
 public:
  /**
   * Class constructor.
   */
  CaloCluster() = default;

  /**
   * Class destructor.
   */
  virtual ~CaloCluster();

  /**
   * Print a description of this object.
   */
  friend std::ostream& operator<<(std::ostream& o, const CaloCluster& d);

  /**
   * Reset the CaloCluster object.
   */
  void clear();

  /**
   * Take in the hits that make up the cluster.
   * @param hit The digi hit's entry number in the events digi
   * collection.
   */
  void addHits(const std::vector<const ldmx::CalorimeterHit*> hitsVec);

  /**
   * Sets total energy for the cluster.
   * @param energy The total energy of the cluster.
   */
  void setEnergy(double energy) { energy_ = energy; }

  /**
   * Sets total number of hits in the cluster.
   * @param nHits The total number of hits_.
   */
  void setNHits(int nHits) { nHits_ = nHits; }

  /**
   * Sets a sorted vector for the IDs of the hits_
   * that make up the cluster.
   * @param IDs Sorted vector of hit IDs.
   */
  void setIDs(std::vector<unsigned int>& hitIDs) { hitIDs_ = hitIDs; }

  void setHitValsX(std::vector<float>& x_) { hitX_ = x_; }
  void setHitValsY(std::vector<float>& x_) { hitY_ = x_; }
  void setHitValsZ(std::vector<float>& x_) { hitZ_ = x_; }
  void setHitValsE(std::vector<float>& x_) { hitE_ = x_; }

  /**
   * Sets the three coordinates of the cluster centroid
   * @param x_ The x_ coordinate.
   * @param y_ The y_ coordinate.
   * @param z_ The z_ coordinate.
   */
  void setCentroidXYZ(double x_, double y_, double z_) {
    centroidX_ = x_;
    centroidY_ = y_;
    centroidZ_ = z_;
  }
  void setRMSXYZ(double x_, double y_, double z_) {
    rmsX_ = x_;
    rmsY_ = y_;
    rmsZ_ = z_;
  }
  void setDXDZ(double x_) { DXDZ_ = x_; }

  void setDYDZ(double x_) { DYDZ_ = x_; }

  void setEDXDZ(double x_) { errDXDZ_ = x_; }

  void setEDYDZ(double x_) { errDYDZ_ = x_; }

  /////////////////////////////////////////////

  // energy of cluster
  double getEnergy() const { return energy_; }

  // number of hits - equivalent to number of strips
  int getNHits() const { return nHits_; }

  // position (weighted by energy)
  double getCentroidX() const { return centroidX_; }
  double getCentroidY() const { return centroidY_; }
  double getCentroidZ() const { return centroidZ_; }
  double getRMSX() const { return rmsX_; }
  double getRMSY() const { return rmsY_; }
  double getRMSZ() const { return rmsZ_; }

  double getDXDZ() const { return DXDZ_; }

  double getDYDZ() const { return DYDZ_; }

  double getEDXDZ() const { return errDXDZ_; }

  double getEDYDZ() const { return errDYDZ_; }

  // get hit rawIDs (unused)
  const std::vector<unsigned int>& getHitIDs() const { return hitIDs_; }

  // ability to store limited hit info
  const std::vector<float>& getHitX() const { return hitX_; }
  const std::vector<float>& getHitY() const { return hitY_; }
  const std::vector<float>& getHitZ() const { return hitZ_; }
  const std::vector<float>& getHitE() const { return hitE_; }

  bool operator<(const CaloCluster& rhs) const {
    return this->getEnergy() < rhs.getEnergy();
  }

 protected:
  std::vector<unsigned int> hitIDs_;
  double energy_{0};
  int nHits_{0};
  double centroidX_{0};
  double centroidY_{0};
  double centroidZ_{0};
  double rmsX_{0};
  double rmsY_{0};
  double rmsZ_{0};
  double DXDZ_{0};
  double DYDZ_{0};
  double errDXDZ_{0};
  double errDYDZ_{0};
  std::vector<float> hitX_;
  std::vector<float> hitY_;
  std::vector<float> hitZ_;
  std::vector<float> hitE_;

 private:
  ClassDef(CaloCluster, 2);
};
}  // namespace ldmx

#endif
