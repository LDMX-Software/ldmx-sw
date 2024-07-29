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
    
    int clusterId;
    std::vector<ldmx::EcalHit> hits;

    Density() {}

    Density(double xx, double yy, double dm) : x(xx), y(yy), delta(dm) {
      totalEnergy = 0.;
      index = -1;
      followerOf = -1;
      clusterId = -1;
      hits = {};
    }
  };

  double dist(double x1, double y1, double x2, double y2) {
    return pow(pow(x1 - x2, 2) + pow(y1 - y2, 2), 0.5);
  }

  void setup(std::vector<ldmx::EcalHit> hits) {
    std::map<std::pair<double, double>, Density> densityMap;
    if (debug_) std::cout << "Building densities" << std::endl;
    for (const auto& hit : hits) {
      // TODO: round x, y to appropriate decimal
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
        // densityMap.emplace(coords, {x, y, 0., -1, -1, dm_, -1, {}});
        // densityMap[coords] = Density{hit.getXPos(), hit.getYPos(), 0., -1, -1, dm_, -1, {}};
      } else if (debug_) std::cout << "    Found density with x: " << densityMap[coords].x << " y: " << densityMap[coords].y << std::endl;
      densityMap[coords].hits.push_back(hit);
      densityMap[coords].totalEnergy += hit.getEnergy();
    }
  
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
      // densities_[i].delta = dm_; // this should be inf, but since we have a condition < dm_ later we try baking it in here
      // densities_[i].followerOf = -1;
      // loop through all higher energy densities
      for (int j = 0; j < i; j++) {
        double d = dist(densities_[i].x, densities_[i].y, densities_[j].x, densities_[j].y);
        // here we also have condition d < dm_, but baked into delta previously
        // + energyJ > energyI but this should be baked in as we sorted according to energy
        if (d < densities_[i].delta) {
          if (debug_) std::cout << "  New parent, index " << j << "; delta: " << d << std::endl;
          densities_[i].delta = d;
          densities_[i].followerOf = j;
        }
      }
    }
  }

  void clustering() {
    int k = 0;
    std::stack<int> clusterStack;
    // stores followers of densities at corr index
    std::vector<std::vector<ldmx::EcalHit>> clusters;
    clusters.reserve(densities_.size());
    std::vector<std::vector<int>> followers;
    followers.resize(densities_.size());
    if (debug_) std::cout << "CLUSTERING" << std::endl;
    for (int i = 0; i < densities_.size(); i++) {
      if (debug_) {
        std::cout << "  Index: " << i << "; x: " << densities_[i].x << "; y: " << densities_[i].y << "; Energy: " << densities_[i].totalEnergy << std::endl;
        std::cout << "  Parent ID: " << densities_[i].followerOf << "; Delta: " << densities_[i].delta << std::endl;
      }
      bool isSeed = densities_[i].totalEnergy > rhoc_ && densities_[i].delta > deltac_;
      bool isOutlier = densities_[i].totalEnergy < rhoc_ && densities_[i].delta > deltao_;
      if (isSeed) {
        if (debug_) std::cout << "  Is seed, cluster id " << k << std::endl;
        densities_[i].clusterId = k;
        k++;
        clusterStack.push(i);
        clusters.push_back(densities_[i].hits);
      } else if (!isOutlier) {
        if (debug_) std::cout << "  Is not outlier" << std::endl;
        int& parentIndex = densities_[i].followerOf;
        if (parentIndex != -1) followers[parentIndex].push_back(i);
      }
    }
    while (clusterStack.size() > 0) {
      auto& d = densities_[clusterStack.top()];
      clusterStack.pop();
      auto& cid = d.clusterId;
      for (const auto& j : followers[d.index]) { // for index of followers of d
        auto& f = densities_[j];
        // set clusterindex of follower to clusterindex of d
        f.clusterId = cid;
        clusters[cid].insert(std::end(clusters[cid]), std::begin(f.hits), std::end(f.hits));
        // add follower to stack, so its followers can also get correct clusterindex
        clusterStack.push(j);
      }
    }

    // Convert to workingecalclusters
    for (const auto& vec : clusters) {
      auto c = WorkingEcalCluster();
      for (const auto& hit : vec) {
        c.add(hit);
      }
      finalClusters_.push_back(c);
    }
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

    setup(hits);
    clustering();

  }

  int getNSeeds() const { return nseeds_; }

  int getNLoops() const { return loops_; }

  std::vector<std::pair<int, double>> getWeights() const { return transitionWeights_; }

  std::vector<WorkingEcalCluster> getClusters() const { return finalClusters_; }

 private:
  int nseeds_;
  int loops_;
  int nClusters_;

  bool debug_;

  double dc_;
  double rhoc_;
  double deltac_;
  double deltao_;
  double dm_;

  std::vector<Density> densities_;

  std::vector<std::pair<int, double>> transitionWeights_;
  std::vector<ldmx::EcalHit> hits_;
  std::vector<WorkingEcalCluster> finalClusters_;
};
}  // namespace ecal

#endif
