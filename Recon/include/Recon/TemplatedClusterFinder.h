/**
 * @file TemplatedClusterFinder.h
 * @brief Templated clustering algorithm for calorimeter hits
 */
#ifndef RECON_TEMPLATEDCLUSTERFINDER_H_
#define RECON_TEMPLATEDCLUSTERFINDER_H_

#include <algorithm>
#include <map>
#include <vector>

#include "Recon/WorkingCluster.h"

namespace recon {

/**
 * @class TemplatedClusterFinder
 * @brief A templated agglomerative clustering algorithm.
 *
 * This class implements a hierarchical agglomerative clustering algorithm
 * for calorimeter hits. It is templated on:
 * - HitType: The type of hit being clustered (must inherit from
 * CalorimeterHit)
 * - WeightClass: A functor that computes the "distance" between two clusters
 *
 * The algorithm works by:
 * 1. Starting with each hit as its own cluster
 * 2. Repeatedly merging the two closest clusters until no pair is closer than
 *    the cutoff
 * 3. Returning clusters that have at least seed_threshold energy
 *
 * @tparam HitType The type of hit (e.g., EcalHit, HcalHit)
 * @tparam WeightClass A functor with operator()(const WorkingCluster<HitType>&,
 *                     const WorkingCluster<HitType>&) -> double
 */
template <class HitType, class WeightClass>
class TemplatedClusterFinder {
 public:
  using ClusterType = WorkingCluster<HitType>;

  /**
   * Add a hit to be clustered using its stored position.
   *
   * @param hit Reference to the hit to add
   */
  void add(const HitType& hit) { clusters_.push_back(ClusterType(hit)); }

  /**
   * Add a hit to be clustered using its stored position.
   *
   * @param hit Pointer to the hit to add
   */
  void add(const HitType* hit) {
    if (hit) {
      clusters_.push_back(ClusterType(hit));
    }
  }

  /**
   * Add a hit with explicit position (e.g., from geometry).
   *
   * @param hit Pointer to the hit to add
   * @param x X position
   * @param y Y position
   * @param z Z position
   */
  void add(const HitType* hit, double x, double y, double z) {
    if (hit) {
      ClusterType cluster;
      cluster.add(hit, x, y, z);
      clusters_.push_back(cluster);
    }
  }

  /**
   * Compare clusters by energy (descending order).
   */
  static bool compClusters(const ClusterType& a, const ClusterType& b) {
    return a.energy() > b.energy();
  }

  /**
   * Run the clustering algorithm.
   *
   * @param seed_threshold Minimum energy for a cluster to be considered a seed
   * @param cutoff Maximum weight (distance) for merging two clusters
   */
  void cluster(double seed_threshold, double cutoff) {
    int ncluster = clusters_.size();
    double minwgt = cutoff;

    // Sort by energy (highest first)
    std::sort(clusters_.begin(), clusters_.end(), compClusters);

    loops_ = 0;
    do {
      bool any = false;
      size_t mi(0), mj(0);
      int nseeds = 0;

      for (size_t i = 0; i < clusters_.size(); i++) {
        if (clusters_[i].empty()) continue;

        bool iseed = (clusters_[i].energy() >= seed_threshold);
        if (iseed) {
          nseeds++;
        } else {
          // Since we sorted initially, if we find a hit below seed threshold
          // there will be no seeds after.
          break;
        }

        for (size_t j = i + 1; j < clusters_.size(); j++) {
          if (clusters_[j].empty() ||
              (!iseed && clusters_[j].energy() < seed_threshold))
            continue;

          double wgt = wgt_(clusters_[i], clusters_[j]);
          if (!any || wgt < minwgt) {
            any = true;
            minwgt = wgt;
            mi = i;
            mj = j;
          }
        }
      }

      nseeds_ = nseeds;
      transition_weights_.insert(std::pair<int, double>(ncluster, minwgt));

      if (any && minwgt < cutoff) {
        // Put the bigger one in mi
        if (clusters_[mi].energy() < clusters_[mj].energy()) {
          std::swap(mi, mj);
        }
        // Merge the smaller into the bigger
        clusters_[mi].add(clusters_[mj]);
        clusters_[mj].clear();
        // Decrement cluster count
        ncluster--;
      }
      loops_++;
    } while (minwgt < cutoff && ncluster > 1);

    finalwgt_ = minwgt;

    // Collect final clusters that pass the seed threshold
    for (const auto& cl : clusters_) {
      if (!cl.empty() && cl.energy() >= seed_threshold) {
        final_clusters_.push_back(cl);
      } else if (cl.energy() < seed_threshold) {
        break;  // Clusters are sorted, so safe to break
      }
    }
  }

  /**
   * Get the final weight (minimum distance between remaining clusters).
   */
  double getYMax() const { return finalwgt_; }

  /**
   * Get the number of seed clusters found.
   */
  int getNSeeds() const { return nseeds_; }

  /**
   * Get the number of iterations performed.
   */
  int getNLoops() const { return loops_; }

  /**
   * Get the transition weights (cluster count -> minimum weight at that step).
   */
  std::map<int, double> getWeights() const { return transition_weights_; }

  /**
   * Get the final clusters after filtering by seed threshold.
   */
  std::vector<ClusterType> getClusters() const { return final_clusters_; }

  /**
   * Get all clusters including empty ones (for debugging).
   */
  std::vector<ClusterType> getAllClusters() const { return clusters_; }

 private:
  WeightClass wgt_;
  double finalwgt_{0};
  int nseeds_{0};
  int loops_{0};
  std::map<int, double> transition_weights_;
  std::vector<ClusterType> clusters_;
  std::vector<ClusterType> final_clusters_;
};

}  // namespace recon

#endif  // RECON_TEMPLATEDCLUSTERFINDER_H_
