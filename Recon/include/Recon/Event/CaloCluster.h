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
  void setNHits(int nHits) { n_hits_ = nHits; }

  /**
   * Sets a sorted vector for the IDs of the hits_
   * that make up the cluster.
   * @param IDs Sorted vector of hit IDs.
   */
  void setIDs(std::vector<unsigned int>& hitIDs) { hit_ids = hitIDs; }

  void setHitValsX(std::vector<float>& x_) { hit_x_ = x_; }
  void setHitValsY(std::vector<float>& x_) { hit_y_ = x_; }
  void setHitValsZ(std::vector<float>& x_) { hit_z_ = x_; }
  void setHitValsE(std::vector<float>& x_) { hit_e_ = x_; }

  /**
   * Sets the three coordinates of the cluster centroid
   * @param x_ The x_ coordinate.
   * @param y_ The y_ coordinate.
   * @param z_ The z_ coordinate.
   */
  void setCentroidXYZ(double x_, double y_, double z_) {
    centroid_x_ = x_;
    centroid_y_ = y_;
    centroid_z_ = z_;
  }
  void setRMSXYZ(double x_, double y_, double z_) {
    rms_x_ = x_;
    rms_y_ = y_;
    rms_z_ = z_;
  }
  void setDXDZ(double x_) { dxdz_ = x_; }

  void setDYDZ(double x_) { dydz_ = x_; }

  void setEDXDZ(double x_) { err_dxdz_ = x_; }

  void setEDYDZ(double x_) { err_dydz_ = x_; }

  /////////////////////////////////////////////

  // energy of cluster
  double getEnergy() const { return energy_; }

  // number of hits - equivalent to number of strips
  int getNHits() const { return n_hits_; }

  // position (weighted by energy)
  double getCentroidX() const { return centroid_x_; }
  double getCentroidY() const { return centroid_y_; }
  double getCentroidZ() const { return centroid_z_; }
  double getRMSX() const { return rms_x_; }
  double getRMSY() const { return rms_y_; }
  double getRMSZ() const { return rms_z_; }

  double getDXDZ() const { return dxdz_; }

  double getDYDZ() const { return dydz_; }

  double getEDXDZ() const { return err_dxdz_; }

  double getEDYDZ() const { return err_dydz_; }

  // get hit rawIDs (unused)
  const std::vector<unsigned int>& getHitIDs() const { return hit_ids; }

  // ability to store limited hit info
  const std::vector<float>& getHitX() const { return hit_x_; }
  const std::vector<float>& getHitY() const { return hit_y_; }
  const std::vector<float>& getHitZ() const { return hit_z_; }
  const std::vector<float>& getHitE() const { return hit_e_; }

  bool operator<(const CaloCluster& rhs) const {
    return this->getEnergy() < rhs.getEnergy();
  }

 protected:
  std::vector<unsigned int> hit_ids;
  double energy_{0};
  int n_hits_{0};
  double centroid_x_{0};
  double centroid_y_{0};
  double centroid_z_{0};
  double rms_x_{0};
  double rms_y_{0};
  double rms_z_{0};
  double dxdz_{0};
  double dydz_{0};
  double err_dxdz_{0};
  double err_dydz_{0};
  std::vector<float> hit_x_;
  std::vector<float> hit_y_;
  std::vector<float> hit_z_;
  std::vector<float> hit_e_;

 private:
  ClassDef(CaloCluster, 2);
};
}  // namespace ldmx

#endif
