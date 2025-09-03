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
   * @param x The x coordinate.
   * @param y The y coordinate.
   * @param z The z coordinate.
   */
  void setCentroidXYZ(double centroid_x, double centroid_y, double centroid_z) {
    centroid_x_ = centroid_x;
    centroid_y_ = centroid_y;
    centroid_z_ = centroid_z;
  }

  // Sets the layer of the cluster centroid
  void setLayer(int layer) { layer_ = layer; }
  void setRMSXYZ(double rms_x, double rms_y, double rms_z) {
    rms_x_ = rms_x;
    rms_y_ = rms_y;
    rms_z_ = rms_z;
  }
  void setDXDZ(double dxdz) { dxdz_ = dxdz; }

  void setDYDZ(double dydz) { dydz_ = dydz; }

  void setEDXDZ(double err_dxdz) { err_dxdz_ = err_dxdz; }

  void setEDYDZ(double err_dydz) { err_dydz_ = err_dydz; }

  /////////////////////////////////////////////

  // energy of cluster
  double getEnergy() const { return energy_; }

  // number of hits - equivalent to number of strips
  int getNHits() const { return n_hits_; }

  // position (weighted by energy)
  /// centroid x-location
  double getCentroidX() const { return centroid_x_; }
  /// @brief  centroid y-location
  double getCentroidY() const { return centroid_y_; }
  /// @brief  centroid z-location
  double getCentroidZ() const { return centroid_z_; }
  /// @brief  layer of the cluster centroid
  int getLayer() const { return layer_; }
  /// @brief  rms in x
  double getRMSX() const { return rms_x_; }
  /// @brief  rms in y
  double getRMSY() const { return rms_y_; }
  /// @brief  rms in z
  double getRMSZ() const { return rms_z_; }
  /// @brief  Delta in x-z plane
  double getDXDZ() const { return dxdz_; }
  /// @brief  Delta in y-z plane
  double getDYDZ() const { return dydz_; }
  /// @brief  Delta unc on unc in x-z plane
  double getEDXDZ() const { return err_dxdz_; }
  /// @brief  Delta unc on unc in y-z plane
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
  int layer_{-1};
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
  ClassDef(CaloCluster, 3);
};
}  // namespace ldmx

#endif
