/**
 * @file WorkingCluster.h
 * @brief In-memory tool for working on clusters during reconstruction
 */
#ifndef RECON_WORKINGCLUSTER_H_
#define RECON_WORKINGCLUSTER_H_

#include <vector>

#include "Recon/Event/CalorimeterHit.h"

namespace recon {

/**
 * @class WorkingCluster
 * @brief An in-memory representation of a cluster being built during
 * reconstruction.
 *
 * This class is a general-purpose tool for building clusters from calorimeter
 * hits. It is templated on the hit type, which must inherit from
 * CalorimeterHit. The cluster stores an energy-weighted centroid in (x,y,z,E)
 * space.
 *
 * @tparam HitType The type of hit to cluster (must inherit from
 * CalorimeterHit)
 */
template <class HitType>
class WorkingCluster {
 public:
  /**
   * Default constructor.
   */
  WorkingCluster() = default;

  /**
   * Construct a cluster from a single hit.
   *
   * @param hit Pointer to the hit to initialize the cluster with
   */
  WorkingCluster(const HitType* hit) {
    if (hit) {
      add(hit);
    }
  }

  /**
   * Construct a cluster from a single hit (reference version).
   *
   * @param hit Reference to the hit to initialize the cluster with
   */
  WorkingCluster(const HitType& hit) { add(hit); }

  /**
   * Default destructor.
   */
  ~WorkingCluster() = default;

  /**
   * Add a hit to the cluster using its stored position.
   *
   * The centroid is updated using energy-weighted averaging.
   *
   * @param hit Pointer to the hit to add
   */
  void add(const HitType* hit) {
    if (!hit) return;

    double hit_e = hit->getEnergy();
    double hit_x = hit->getXPos();
    double hit_y = hit->getYPos();
    double hit_z = hit->getZPos();
    double hit_t = hit->getTime();

    double new_e = hit_e + centroid_e_;
    if (new_e > 0) {
      centroid_x_ = (centroid_x_ * centroid_e_ + hit_e * hit_x) / new_e;
      centroid_y_ = (centroid_y_ * centroid_e_ + hit_e * hit_y) / new_e;
      centroid_z_ = (centroid_z_ * centroid_e_ + hit_e * hit_z) / new_e;
    }
    centroid_e_ = new_e;

    // Track the latest time
    if (hit_t > time_) {
      time_ = hit_t;
    }

    hits_.push_back(hit);
  }

  /**
   * Add a hit to the cluster using its stored position (reference version).
   *
   * @param hit Reference to the hit to add
   */
  void add(const HitType& hit) { add(&hit); }

  /**
   * Add a hit to the cluster with explicit position.
   *
   * This version allows providing the position externally (e.g., from
   * geometry).
   *
   * @param hit Pointer to the hit to add
   * @param x X position of the hit
   * @param y Y position of the hit
   * @param z Z position of the hit
   */
  void add(const HitType* hit, double x, double y, double z) {
    if (!hit) return;

    double hit_e = hit->getEnergy();
    double hit_t = hit->getTime();

    double new_e = hit_e + centroid_e_;
    if (new_e > 0) {
      centroid_x_ = (centroid_x_ * centroid_e_ + hit_e * x) / new_e;
      centroid_y_ = (centroid_y_ * centroid_e_ + hit_e * y) / new_e;
      centroid_z_ = (centroid_z_ * centroid_e_ + hit_e * z) / new_e;
    }
    centroid_e_ = new_e;

    if (hit_t > time_) {
      time_ = hit_t;
    }

    hits_.push_back(hit);
  }

  /**
   * Merge another cluster into this one.
   *
   * @param other The cluster to merge into this one
   */
  void add(const WorkingCluster<HitType>& other) {
    double new_e = other.centroid_e_ + centroid_e_;
    if (new_e > 0) {
      centroid_x_ =
          (centroid_x_ * centroid_e_ + other.centroid_x_ * other.centroid_e_) /
          new_e;
      centroid_y_ =
          (centroid_y_ * centroid_e_ + other.centroid_y_ * other.centroid_e_) /
          new_e;
      centroid_z_ =
          (centroid_z_ * centroid_e_ + other.centroid_z_ * other.centroid_e_) /
          new_e;
    }
    centroid_e_ = new_e;

    if (other.time_ > time_) {
      time_ = other.time_;
    }

    for (const auto* hit : other.hits_) {
      hits_.push_back(hit);
    }
  }

  /**
   * Get the centroid X position (energy-weighted).
   */
  double centroidX() const { return centroid_x_; }

  /**
   * Get the centroid Y position (energy-weighted).
   */
  double centroidY() const { return centroid_y_; }

  /**
   * Get the centroid Z position (energy-weighted).
   */
  double centroidZ() const { return centroid_z_; }

  /**
   * Get the total energy of the cluster.
   */
  double energy() const { return centroid_e_; }

  /**
   * Get the time of the cluster (latest hit time).
   */
  double time() const { return time_; }

  /**
   * Get the list of hits in this cluster.
   */
  const std::vector<const HitType*>& hits() const { return hits_; }

  /**
   * Check if the cluster is empty.
   */
  bool empty() const { return hits_.empty(); }

  /**
   * Clear all hits from the cluster.
   */
  void clear() {
    hits_.clear();
    centroid_x_ = 0;
    centroid_y_ = 0;
    centroid_z_ = 0;
    centroid_e_ = 0;
    time_ = 0;
  }

  /**
   * Set the centroid position and energy explicitly.
   */
  void setCentroid(double x, double y, double z, double e) {
    centroid_x_ = x;
    centroid_y_ = y;
    centroid_z_ = z;
    centroid_e_ = e;
  }

  /**
   * Set the time explicitly.
   */
  void setTime(double t) { time_ = t; }

  /**
   * Get the layer of the cluster.
   */
  int layer() const { return layer_; }

  /**
   * Set the layer of the cluster.
   */
  void setLayer(int layer) { layer_ = layer; }

 private:
  /** The hits in this cluster */
  std::vector<const HitType*> hits_;

  /** Centroid X position (energy-weighted) */
  double centroid_x_{0};

  /** Centroid Y position (energy-weighted) */
  double centroid_y_{0};

  /** Centroid Z position (energy-weighted) */
  double centroid_z_{0};

  /** Total energy */
  double centroid_e_{0};

  /** Time (latest hit time) */
  double time_{0};

  /** Layer number */
  int layer_{-1};
};

}  // namespace recon

#endif  // RECON_WORKINGCLUSTER_H_
