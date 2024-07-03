/*
   TemplatedEcalClusterFinder
   */

#ifndef ECAL_TEMPLATEDECALCLUSTERFINDER_H_
#define ECAL_TEMPLATEDECALCLUSTERFINDER_H_

#include <math.h>
#include <algorithm>
#include <unordered_map>
#include <iostream>

#include "Ecal/WorkingEcalCluster.h"

#include <iostream>

namespace ecal {

template <class WeightClass>

class TemplatedEcalClusterFinder {
 public:

  void createCentroid(const ldmx::EcalHit& eh) {
    centroids_.insert({eh.getID(), WorkingEcalCluster(eh)});
  }

  static bool compHits(const ldmx::EcalHit& a, const ldmx::EcalHit& b) {
    return a.getEnergy() > b.getEnergy();
  }

  void recursive_clustering(const ldmx::EcalHit& current_hit, WorkingEcalCluster& centroid, std::vector<int>& path) {
    depth_++;
    for (auto& hit : hits_) {
      if (hit.getID() == current_hit.getID()) continue;
      if (std::find(path.begin(), path.end(), hit.getID()) != path.end()) continue;
      double wgt = wgt_(current_hit, hit);
      if (wgt < neighbourWeight_) {
        if (hit.getEnergy() >= cellFilter_) {
          path.push_back(hit.getID());
          transitionWeights_.push_back({nClusters_, wgt});
        }
        if (hit.getEnergy() >= seedThreshold_) {
          // hit is seed, merge clusters
          // find centroid 
          auto it = centroids_.find(hit.getID());
          if (it != centroids_.end()) {
            auto& mergeCentroid = it->second; // WorkingEcalCluster
            int size = mergeCentroid.getHits().size();
            for (const auto& mhit : mergeCentroid.getHits()) {
              path.push_back(mhit.getID());
            }
            centroid.add(mergeCentroid);
            mergeCentroid.clear();
            nClusters_--;
            // if centroid only has seed in it, explore neighbours
            // otherwise seed has already been looped through
            if (size == 1) recursive_clustering(hit, centroid, path);
          }
        } else if (hit.getEnergy() >= growthThreshold_) {
          centroid.add(hit);
          recursive_clustering(hit, centroid, path);
        } else if (hit.getEnergy() >= cellFilter_) {
          centroid.add(hit);
        }
      }
    }
    loops_++;
  }

  void cluster(std::vector<ldmx::EcalHit>& ecalHits,
                          double seed, double growth, double filter, double neighbour_wgt) {
    hits_ = ecalHits;
    seedThreshold_ = seed;
    growthThreshold_ = growth;
    cellFilter_ = filter;
    neighbourWeight_ = neighbour_wgt;
    // Sort after highest energy
    std::sort(hits_.begin(), hits_.end(), compHits);
    // Find seeds
    for (const auto& hit : hits_) {
      if (hit.getEnergy() >= seedThreshold_) {
        createCentroid(hit);
      } else break;
    }
    nClusters_ = centroids_.size();
    loops_ = 0;
    for (auto& [id, centroid]: centroids_) {
      if (centroid.empty()) continue;
      std::vector<int> path;
      path.push_back(centroid.getHits()[0].getID());
      recursive_clustering(centroid.getHits()[0], centroid, path);
    }
    
    for (auto& [id, centroid]: centroids_) {
      if (!centroid.empty()) {
        finalClusters_.push_back(centroid);
      }
    }
  }

  int getNSeeds() const { return nseeds_; }

  int getNLoops() const { return loops_; }

  std::vector<std::pair<int, double>> getWeights() const { return transitionWeights_; }

  std::vector<WorkingEcalCluster> getClusters() const { return finalClusters_; }

 private:
  WeightClass wgt_;
  double finalwgt_;
  int nseeds_;
  int loops_;
  int depth_;
  int nClusters_;

  double neighbourWeight_;
  double seedThreshold_;
  double growthThreshold_;
  double cellFilter_;

  std::vector<std::pair<int, double>> transitionWeights_;
  std::vector<ldmx::EcalHit> hits_;
  std::unordered_map<int, WorkingEcalCluster> centroids_;
  std::vector<WorkingEcalCluster> finalClusters_;
};
}  // namespace ecal

#endif
