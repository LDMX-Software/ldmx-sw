/*
   CLUE
   */

#ifndef ECAL_CLUE_H_
#define ECAL_CLUE_H_

#include <math.h>
#include <algorithm>
#include <map>
#include <iostream>
#include <stack>
#include <limits>
#include <set>

#include "Ecal/WorkingEcalCluster.h"
#include "Ecal/Event/EcalHit.h"

#include <iostream>

namespace ecal {

class CLUE {
 public:

  struct Density {
    double x;
    double y;
    double totalEnergy;
    int index;

    // index of density this density is follower of
    // set to index of spatially closest density with higher energy; -1 if seed
    int followerOf;
    // separation distance to density that this is follower of
    double delta;
    
    bool overloaded;
    int clusterId;
    std::vector<ldmx::EcalHit> hits;

    Density() {}

    Density(double xx, double yy, double dm) : x(xx), y(yy) {
      totalEnergy = 0.;
      index = -1;
      followerOf = -1;
      delta = std::numeric_limits<double>::max();
      overloaded = false;
      clusterId = -1;
      hits = {};
    }
  };

  double dist(double x1, double y1, double x2, double y2) {
    return pow(pow(x1 - x2, 2) + pow(y1 - y2, 2), 0.5);
  }

  // void electronSeparation(std::vector<ldmx::EcalHit> hits) {
  //   std::vector<double> layerThickness = { 2., 3.5, 5.3, 5.3, 5.3, 5.3, 5.3, 5.3, 5.3, 5.3, 5.3, 10.5, 10.5, 10.5, 10.5, 10.5 };
  //   double air = 10.;
  //   std::sort(hits.begin(), hits.end(), [](const ldmx::EcalHit& a, const ldmx::EcalHit& b) {
  //       return a.getZPos() < b.getZPos();
  //   });
  //   std::vector<ldmx::EcalHit> firstLayers;
  //   std::vector<WorkingEcalCluster> firstLayerClusters;
  //   int layerTag = 0;
  //   double layerZ = hits[0].getZPos();
  //   for (const auto& hit : hits) {
  //     if (hit.getZPos() > layerZ + layerThickness[layerTag] + air) {
  //       layerTag++;
  //       // if (layerTag > limit) break;
  //       break;
  //     }
  //     firstLayers.push_back(hit);
  //     firstLayerClusters.push_back(WorkingEcalCluster(hit, layerTag));
      
  //   }
  //   bool merge = false;
  //   do {
  //     merge = false;
  //     for (int i = 0; i < firstLayerClusters.size(); i++) {
  //       if (firstLayerClusters[i].empty()) continue;
  //       // if (firstLayerClusters[i].centroid().E() >= seedThreshold_) {
  //       for (int j = i + 1; j < firstLayerClusters.size(); j++) {
  //         if (firstLayerClusters[j].empty()) continue;
  //         if (dist(firstLayerClusters[i].centroid().Px(), firstLayerClusters[i].centroid().Py(), firstLayerClusters[j].centroid().Px(), firstLayerClusters[j].centroid().Py()) < 8.) {
  //           firstLayerClusters[i].add(firstLayerClusters[j]);
  //           firstLayerClusters[j].clear();
  //           merge = true;
  //         }
  //       }
  //       // } else break;
  //     }
  //   } while (merge);
  //   if (debug_) std::cout << "--- ELECTRON SEPARATION ---" << std::endl;
  //   for (int i = 0; i < firstLayerClusters.size(); i++) {
  //     if (firstLayerClusters[i].empty()) continue;
  //     if (debug_) std::cout << "  Cluster " << i << " x: " << firstLayerClusters[i].centroid().Px() << " y: " << firstLayerClusters[i].centroid().Py() << std::endl;
  //     for (int j = i + 1; j < firstLayerClusters.size(); j++) {
  //       if (firstLayerClusters[j].empty()) continue;
  //       auto d = dist(firstLayerClusters[i].centroid().Px(), firstLayerClusters[i].centroid().Py(), firstLayerClusters[j].centroid().Px(), firstLayerClusters[j].centroid().Py());
  //       if (debug_) std::cout << "    Dist to cluster " << j << ": " << d << std::endl;
  //     }
  //   }
  // }

  void setup(std::vector<ldmx::EcalHit> hits) {
    std::map<std::pair<double, double>, Density> densityMap;
    centroid_ = WorkingEcalCluster();
    if (debug_) std::cout << "--- SETUP ---" << std::endl << "Building densities" << std::endl;
    for (const auto& hit : hits) {
      // TODO: round x, y to appropriate decimal? Works fine as is
      // collapse z dimension
      double x = static_cast<double>(hit.getXPos());
      double y = static_cast<double>(hit.getYPos());
      if (debug_) {
        std::cout << "  New hit" << std::endl;
        std::cout << "    x: " << x << std::endl;
        std::cout << "    y: " << y << std::endl;
      }
      std::pair<double, double> coords = {x, y};
      if (densityMap.find(coords) == densityMap.end()) {
        densityMap.emplace(coords, Density(x, y, dm_));
        if (debug_) std::cout << "    New density created" << std::endl;
      } else if (debug_) std::cout << "    Found density with x: " << densityMap[coords].x << " y: " << densityMap[coords].y << std::endl;
      densityMap[coords].hits.push_back(hit);
      densityMap[coords].totalEnergy += hit.getEnergy();
      centroid_.add(hit);
    }

    // if (debug_) finalClusters_.push_back(centroid_);
  
    // sort according to energy
    densities_.reserve(densityMap.size());
    for (const auto& entry : densityMap) {
        densities_.push_back(entry.second);
    }
    std::sort(densities_.begin(), densities_.end(), [](const Density& a, const Density& b) {
        return a.totalEnergy > b.totalEnergy;
    });
    
    if (debug_) std::cout << "Decide parents" << std::endl;

    // decide delta and followerOf
    for (int i = 0; i < densities_.size(); i++) {
      densities_[i].index = i;
      if (debug_) std::cout << "  Index: " << i << "; x: " << densities_[i].x << "; y: " << densities_[i].y << "; Energy: " << densities_[i].totalEnergy << std::endl;
      // loop through all higher energy densities
      for (int j = 0; j < i; j++) {
        double d = dist(densities_[i].x, densities_[i].y, densities_[j].x, densities_[j].y);
        // condition energyJ > energyI but this should be baked in as we sorted according to energy
        if (d < dm_ && d < densities_[i].delta) {
          if (debug_) std::cout << "  New parent, index " << j << "; delta: " << d << std::endl;
          densities_[i].delta = d;
          densities_[i].followerOf = j;
        }
      }
    }
  }

  void clustering() {
    if (debug_) std::cout << "--- CLUSTERING ---" << std::endl;
// stores followers of densities at corr index
    bool energyOverload = false;
    double maxEnergy = 10000.;
    clusteringLoops_ = 0;
    double deltacMod = deltac_;
    double centroidRadius = 10.;

    std::vector<std::vector<ldmx::EcalHit>> clusters;
    std::vector<bool> mergedDensities; // index = cluster id
    mergedDensities.resize(densities_.size());
    do {
      if (energyOverload) {
        deltacMod = deltacMod/1.1;
        if (debug_) std::cout << "  Energy overload, new deltacmod: " << deltacMod << std::endl;
        energyOverload = false;
      }

      clusteringLoops_++;
      if (debug_) std::cout << "Clustering loop " << clusteringLoops_ << std::endl;

      int k = 0;

      std::stack<int> clusterStack;
      clusters.clear();
      clusters.reserve(densities_.size());
      std::vector<double> clusterEnergies;
      clusterEnergies.reserve(densities_.size());
      // stores followers of densities at corr index
      std::vector<std::vector<int>> followers;
      followers.resize(densities_.size());
      
      for (int i = 0; i < densities_.size(); i++) {
        if (debug_) {
          std::cout << "  Index: " << i << "; x: " << densities_[i].x << "; y: " << densities_[i].y << "; Energy: " << densities_[i].totalEnergy << std::endl;
          std::cout << "  Parent ID: " << densities_[i].followerOf << "; Delta: " << densities_[i].delta << std::endl;
        }

        bool isSeed;
        // if energy has been overloaded and this density belongs to cluster that was overloaded and this density is close enough to event centroid
        if (deltacMod != deltac_ && mergedDensities[densities_[i].clusterId] 
            && (densities_[i].x, densities_[i].y, centroid_.centroid().Px(), centroid_.centroid().Py()) < centroidRadius) {
          isSeed = densities_[i].totalEnergy > rhoc_ && densities_[i].delta > deltacMod;
        } else isSeed = densities_[i].totalEnergy > rhoc_ && densities_[i].delta > deltac_;
        if (debug_ && isSeed) std::cout << "  Distance to centroid: " 
                                        << dist(densities_[i].x, densities_[i].y, centroid_.centroid().Px(), centroid_.centroid().Py()) 
                                        << std::endl;
        
        bool isOutlier = densities_[i].totalEnergy < rhoc_ && densities_[i].delta > deltao_;

        densities_[i].clusterId = -1;
        if (isSeed) {
          if (debug_) std::cout << "  SEED, cluster id " << k << std::endl;
          densities_[i].clusterId = k;
          k++;
          clusterStack.push(i);
          clusters.push_back(densities_[i].hits);
          clusterEnergies.push_back(densities_[i].totalEnergy);
        } else if (!isOutlier) {
          if (debug_) std::cout << "  Follower" << std::endl;
          int& parentIndex = densities_[i].followerOf;
          if (parentIndex != -1) followers[parentIndex].push_back(i);
          else if (debug_) std::cout << "  HAS PARENT ID -1" << std::endl; // should not happen
        } else if (debug_) std::cout << "  Outlier" << std::endl;
      }

      mergedDensities.clear();
      mergedDensities.resize(densities_.size());

      while (clusterStack.size() > 0) {
        auto& d = densities_[clusterStack.top()];
        clusterStack.pop();
        auto& cid = d.clusterId;
        for (const auto& j : followers[d.index]) { // for indices of followers of d
          auto& f = densities_[j];
          // set clusterindex of follower to clusterindex of d
          f.clusterId = cid;
          clusterEnergies[cid] += f.totalEnergy;
          if (clusterEnergies[cid] > maxEnergy && clusteringLoops_ < 100) {
            mergedDensities[cid] = true;
            if (!energyOverload && clusteringLoops_ == 99) std::cout << "Merged clusters, max cluster loops hit" << std::endl;
            energyOverload = true;
            goto endwhile;
          }
          clusters[cid].insert(std::end(clusters[cid]), std::begin(f.hits), std::end(f.hits));
          // add follower to stack, so its followers can also get correct clusterindex
          clusterStack.push(j);
        }
      } 
      endwhile:;
    } while (energyOverload);

    // Convert to workingecalclusters
    for (const auto& vec : clusters) {
      auto c = WorkingEcalCluster();
      for (const auto& hit : vec) {
        c.add(hit);
      }
      finalClusters_.push_back(c);
      const auto& d = dist(c.centroid().Px(), c.centroid().Py(), centroid_.centroid().Px(), centroid_.centroid().Py());
      avgCentroidDistance_ += d;
      centroidDistances_.push_back(d);
    }
    avgCentroidDistance_ = avgCentroidDistance_/finalClusters_.size();
  }

  void cluster(std::vector<ldmx::EcalHit> hits, double dc, double rc, double deltac, double deltao, bool debug) {
    // cutoff distance for local density
    // currently not used
    dc_ = dc;
    // min density to promote as seed/max density to demote as outlier
    rhoc_ = rc;
    // min separation distance for seeds
    deltac_ = deltac;
    // min separation distance for outliers
    deltao_ = deltao;
    dm_ = std::max(deltac, deltao);

    debug_ = debug;

    // electronSeparation(hits);
    setup(hits);
    clustering();

  }

  double getAvgCentroidDistance() const { return avgCentroidDistance_; }

  std::vector<double> getCentroidDistances() const { return centroidDistances_; }

  int getNLoops() const { return clusteringLoops_; }

  std::vector<WorkingEcalCluster> getClusters() const { return finalClusters_; }

 private:
  int clusteringLoops_;

  bool debug_;

  double dc_;
  double rhoc_;
  double deltac_;
  double deltao_;
  double dm_;

  double avgCentroidDistance_;
  std::vector<double> centroidDistances_;
  WorkingEcalCluster centroid_;

  std::vector<Density> densities_;
  std::vector<WorkingEcalCluster> finalClusters_;
};
}  // namespace ecal

#endif
