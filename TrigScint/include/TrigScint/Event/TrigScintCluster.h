
#ifndef TRIGSCINT_EVENT_TRIGSCINTCLUSTER_H_
#define TRIGSCINT_EVENT_TRIGSCINTCLUSTER_H_

// STL
#include <iostream>
#include <set>

// ldmx-sw
#include "Recon/Event/EventConstants.h"
#include "TrigScint/Event/TestBeamHit.h"
#include "TrigScint/Event/TrigScintHit.h"

namespace ldmx {

/**
 * @class TrigScintCluster
 * @brief Stores cluster information from the trigger scintillator pads. Adds on
 * the ECal cluster functionality
 */
class TrigScintCluster {
 public:
  /**
   * Class constructor.
   */
  TrigScintCluster() = default;

  /**
   * Class destructor.
   */
  virtual ~TrigScintCluster();

  /**
   * Print a description of this object.
   */
  void Print(Option_t *option = "") const;  // override;

  /**
   * Reset the TrigScintCluster object.
   */
  void Clear(Option_t *option = "");  // override;

  /**
   * Take in the hits that make up the cluster.
   * @param idx The digi hit's entry number in the event's digi
   * collection.
   * @param hit The digi hit
   */

  void addHit(uint idx, const ldmx::TrigScintHit *hit);

  /**
   * @param idx The digi collection index of the hit seeding the cluster
   */
  void setSeed(int idx) { seed_ = idx; }

  /**
   *Set the cluster energy
   * @param energy The cluster energy deposition (in units given by hit)
   */
  void setEnergy(double energy) { energy_ = energy; }

  /**
   *Set the cluster photoelectron count (PE)
   * @param PE The cluster photoelectron count
   */
  void setPE(float PE) { PE_ = PE; }

  /**
   *The number of hits forming the cluster
   * @param nHits Number of hits forming the cluster
   */
  void setNHits(int nHits) { nHits_ = nHits; }

  /**
   *The channel numbers of hits forming the cluster
   * @param hitIDs vector of channel numbers of hits forming the cluster
   */
  void setIDs(std::vector<unsigned int> &hitIDs) { hitIDs_ = hitIDs; }

  /**
   *The cluster centroid position in x,y,z (barID space)
   * @param cx Cluster x coordinate
   * @param cy Cluster y coordinate
   * @param cz Cluster z coordinate
   */
  void setCentroidXYZ(double cx, double cy, double cz) {
    centroidX_ = cx;
    centroidY_ = cy;
    centroidZ_ = cz;
  }

  /**
   *The cluster centroid position in x,y,z (real space)
   * @param x Cluster x coordinate
   * @param y Cluster y coordinate
   * @param z Cluster z coordinate
   */
  void setPositionXYZ(double x, double y, double z) {
    x_ = x;
    y_ = y;
    z_ = z;
  }

  /**
   * @param centroid The channel ID centroid
   */
  void setCentroid(double centroid) { centroid_ = centroid; }

  /** Set time of hit. */
  void setTime(float t) { time_ = t; }

  /** Get time of hit. */
  float getTime() const { return time_; }

  /** Set beam energy fraction of hit. */
  void setBeamEfrac(float e) { beamEfrac_ = e; }

  /** Get beam energy fraction of hit. */
  float getBeamEfrac() const { return beamEfrac_; }

  /** Get cluster seed channel nb */
  int getSeed() const { return seed_; }

  /** Get cluster total energy deposition */
  double getEnergy() const { return energy_; }

  /** Get cluster total photoelectron count */
  double getPE() const { return PE_; }

  /** Get the number of hits constituting the cluster */
  int getNHits() const { return nHits_; }

  /** Get cluster centroid in x [mm] (not implmented) */
  double getCentroidX() const { return centroidX_; }

  /** Get cluster centroid in y [mm] (not implmented) */
  double getCentroidY() const { return centroidY_; }

  /** Get cluster centroid in z [mm] (not implmented) */
  double getCentroidZ() const { return centroidZ_; }

  /** Get cluster centroid in x [mm] */
  double getPositionX() const {return x_;}

  /** Get cluster centroid in y [mm] */
  double getPositionY() const {return y_;}

  /** Get cluster centroid in z [mm] */
  double getPositionZ() const {return z_;}

  /** Get vector of channel IDs of hits forming the cluster */
  const std::vector<unsigned int> &getHitIDs() const { return hitIDs_; }

  /** Get the cluster centroid in units of channel nb */
  double getCentroid() const { return centroid_; }

  bool operator<(const TrigScintCluster &rhs) const {
    return this->getEnergy() < rhs.getEnergy();
  }

 private:
  // hits forming the cluster
  std::vector<unsigned int> hitIDs_;

  // total cluster energy depostion
  double energy_{0};

  // number of hits forming the cluster
  int nHits_{0};

  // total cluster photoelectron count
  float PE_{0};

  // index of cluster seeding hit
  int seed_{-1};

  // hit centroid in units of channel nb: energy weighted average of the IDs of
  // the hits forming the cluster
  double centroid_{-1};

  // Centroid x position in barID space (not implemented)
  double centroidX_{-1};

  // Centroid y position in barID space (not implemented)
  double centroidY_{-1};

  // Centroid z position in barID space (not implemented)
  double centroidZ_{-1};

  // Centroid position in x [mm]
  double x_{-99999.};

  // Centroid position in y [mm]
  double y_{-99999.};

  // Centroid position in z [mm]
  double z_{-99999.};

  // fraction of cluster energy deposited in a sim hit associated with beam
  // electrons
  float beamEfrac_{0.};

  // cluster time: energy weighted average of the times of the hits forming the
  // cluster
  float time_{0.};

  /**
   * The ROOT class definition.
   */
  ClassDef(TrigScintCluster, 1);
};
}  // namespace ldmx

#endif  // TRIGSCINT_EVENT_TRIGSCINTCLUSTER_H_
