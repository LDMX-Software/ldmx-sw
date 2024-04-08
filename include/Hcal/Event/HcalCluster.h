/**
 * @file HcalCluster.h
 * @brief Class that stores cluster information from the ECal
 * @author Josh Hiltbrand, University of Minnesota
 * @author Soophie Middleton, Caltech
 */

#ifndef EVENT_HCALCLUSTER_H_
#define EVENT_HCALCLUSTER_H_

/* // ROOT */
/* #include "TObject.h"  //For ClassDef */
/* #include "TString.h" */

/* // STL */
/* #include <iostream> */
/* #include <set> */

// ldmx-sw
#include "Hcal/Event/HcalHit.h"
#include "Recon/Event/CaloCluster.h"

namespace ldmx {

/**
 * @class HcalCluster
 * @brief Stores cluster information from the HCal
 */
class HcalCluster : public ldmx::CaloCluster {
 public:
  /**
   * Class constructor.
   */
  HcalCluster() = default;

  /**
   * Class destructor.
   */
  virtual ~HcalCluster();

  /**
   * Print a description of this object.
   */
  //void Print() const;

  /**
   * Reset the HcalCluster object.
   */
  void Clear();

  /**
   * Take in the hits that make up the cluster.
   * @param hit The digi hit's entry number in the events digi
   * collection.
   */
  void addHits(const std::vector<const ldmx::HcalHit*> hitsVec);

  /* /\** */
  /*  * Sets total energy for the cluster. */
  /*  * @param energy The total energy of the cluster. */
  /*  *\/ */
  /* void setEnergy(double energy) { energy_ = energy; } */

  /* /\** */
  /*  * Sets total number of hits in the cluster. */
  /*  * @param nHits The total number of hits. */
  /*  *\/ */
  /* void setNHits(int nHits) { nHits_ = nHits; } */

  /* /\** */
  /*  * Sets a sorted vector for the IDs of the hits */
  /*  * that make up the cluster. */
  /*  * @param IDs Sorted vector of hit IDs. */
  /*  *\/ */
  /* void setIDs(std::vector<unsigned int>& hitIDs) { hitIDs_ = hitIDs; } */

  /* /\** */
  /*  * Sets the three coordinates of the cluster centroid */
  /*  * @param x The x coordinate. */
  /*  * @param y The y coordinate. */
  /*  * @param z The z coordinate. */
  /*  *\/ */
  /* void setCentroidXYZ(double x, double y, double z) { */
  /*   centroidX_ = x; */
  /*   centroidY_ = y; */
  /*   centroidZ_ = z; */
  /* } */
  /* void setRMSXYZ(double x, double y, double z) { */
  /*   rmsX_ = x; */
  /*   rmsY_ = y; */
  /*   rmsZ_ = z; */
  /* } */
  /* void setDXDZ(double x) { DXDZ_=x; } */

  /* void setDYDZ(double x) { DYDZ_=x; } */

  /* void setEDXDZ(double x) { errDXDZ_=x; } */

  /* void setEDYDZ(double x) { errDYDZ_=x; } */

  void setTime(double x) { time_ = x; }

  /* double getEnergy() const { return energy_; } */

  /* int getNHits() const { return nHits_; } */

  /* double getCentroidX() const { return centroidX_; } */

  /* double getCentroidY() const { return centroidY_; } */

  /* double getCentroidZ() const { return centroidZ_; } */

  /* double getRMSX() const { return rmsX_; } */

  /* double getRMSY() const { return rmsY_; } */

  /* double getRMSZ() const { return rmsZ_; } */

  /* double getDXDZ() const { return DXDZ_; } */

  /* double getDYDZ() const { return DYDZ_; } */

  /* double getEDXDZ() const { return errDXDZ_; } */

  /* double getEDYDZ() const { return errDYDZ_; } */

  double getTime() const { return time_; }

  /* const std::vector<unsigned int>& getHitIDs() const { return hitIDs_; } */

  bool operator<(const HcalCluster& rhs) const {
    return this->getEnergy() < rhs.getEnergy();
  }

 private:
  /* std::vector<unsigned int> hitIDs_; */
  /* double energy_{0}; */
  /* int nHits_{0}; */
  /* double centroidX_{0}; */
  /* double centroidY_{0}; */
  /* double centroidZ_{0}; */
  /* double rmsX_{0}; */
  /* double rmsY_{0}; */
  /* double rmsZ_{0}; */
  /* double DXDZ_{0}; */
  /* double DYDZ_{0}; */
  /* double errDXDZ_{0}; */
  /* double errDYDZ_{0}; */
  double time_{0};

  ClassDef(HcalCluster, 1);
};
}  // namespace ldmx

#endif
