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
#include <memory>
#include <iomanip>

#include "Ecal/WorkingEcalCluster.h"
#include "Ecal/Event/EcalHit.h"

#include <iostream>

namespace ecal {

class CLUE {
 public:

  struct Density {
    double x;
    double y;
    float fx;
    float fy;
    double totalEnergy;
    int index;

    // index of density this density is follower of
    // set to index of spatially closest density with higher energy; -1 if seed
    int followerOf;
    // separation distance to density that this is follower of
    double delta;
    float fDelta;

    // index of density this follower is second closest to
    int secondFollowerOf;
    // separation distance to second closest density
    double secondDelta;
    float fSecondDelta;

    int clusterId;
    int secondClusterId;
    // distance percentage to cluster 1 (if mixed)
    double c1p;
    // distance percentage to cluster 2 (if mixed)
    double c2p;

    std::vector<ldmx::EcalHit> hits;

    Density() {}

    Density(double xx, double yy, float fxx, float fyy) : x(xx), y(yy), fx(fxx), fy(fyy) {
      totalEnergy = 0.;
      index = -1;
      followerOf = -1;
      delta = std::numeric_limits<double>::max();
      fDelta = std::numeric_limits<float>::max();
      secondFollowerOf = -1;
      secondDelta = std::numeric_limits<double>::max();
      fSecondDelta = std::numeric_limits<float>::max();
      clusterId = -1;
      secondClusterId = -1;
      hits = {};
    }
  };

  double dist(double x1, double y1, double x2, double y2) {
    return pow(pow(x1 - x2, 2) + pow(y1 - y2, 2), 0.5);
  }

  float floatDist(float x1, float y1, float x2, float y2) {
    return powf(powf(x1 - x2, 2) + powf(y1 - y2, 2), 0.5);
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

  // double rhocFactor(int trueLayer) {
  //   return trueLayer*trueLayer - 
  // }

  std::vector<std::vector<ldmx::EcalHit>> createLayers(std::vector<ldmx::EcalHit>& hits) {
    if (debug_) std::cout << "--- LAYER CREATION ---" << std::endl;
    std::vector<std::vector<ldmx::EcalHit>> layers;
    std::sort(hits.begin(), hits.end(), [](const ldmx::EcalHit& a, const ldmx::EcalHit& b) {
      return a.getZPos() < b.getZPos();
    });
    int layerTag = 0;
    int trueLayer = 0;
    double layerZ = hits[0].getZPos();
    double trueLayerZ = layerZ;
    double maxZ = hits[hits.size()-1].getZPos();
    layers.push_back({});
    double layerSeparation = (maxZ-layerZ)/nbrOfLayers_;
    if (debug_) std::cout << "  Layer separation: " << layerSeparation << std::endl << "  Creating layer 0" << std::endl;

    double highestEnergy = 0.;
    int rhocFactor = 2.;
    for (const auto& hit : hits) {      
      // If z of hit is in new layer, both calculated and real (we don't want to split in the middle of actual ecal layer)
      if (layerTag != nbrOfLayers_ && hit.getZPos() > (layerZ + layerSeparation) && hit.getZPos() > trueLayerZ + layerThickness_[trueLayer] + air_) {
        layerZ = hit.getZPos();
        layers.push_back({ });
        // Set seed threshold for layer to highest energy of layer / factor
        layerRhoC_.push_back(highestEnergy/rhocFactor); // TODO: decide division factor
        layerTag++;
        if (debug_) std::cout << "    Highest energy: " << highestEnergy << std::endl << "  Creating layer " << layerTag << std::endl;
        highestEnergy = 0.;
      }
      if (hit.getZPos() > trueLayerZ + layerThickness_[trueLayer] + air_) {
        trueLayer++;
        trueLayerZ = hit.getZPos();
      }
      layers[layerTag].push_back(hit);
      if (hit.getEnergy() > highestEnergy) highestEnergy = hit.getEnergy();
    }
    layerRhoC_.push_back(highestEnergy/rhocFactor);
    return layers;
  }

  float roundToDecimal(float x, int num_decimal_precision_digits) {
      float power_of_10 = std::pow(10, num_decimal_precision_digits);
      return std::round(x * power_of_10)  / power_of_10;
  }

  std::vector<std::shared_ptr<Density>> setup(const std::vector<ldmx::EcalHit>& hits) {
    std::vector<std::shared_ptr<Density>> densities;
    std::map<std::pair<float, float>, std::shared_ptr<Density>> densityMap;
    centroid_ = WorkingEcalCluster();
    if (debug_) std::cout << "--- SETUP ---" << std::endl << "Building densities" << std::endl;
    for (const auto& hit : hits) {
      // TODO: round x, y to appropriate decimal? Works fine as is
      // collapse z dimension
      double x = static_cast<double>(hit.getXPos());
      double y = static_cast<double>(hit.getYPos());
      float fx = roundToDecimal(hit.getXPos(), 4);
      float fy = roundToDecimal(hit.getYPos(), 4);
      if (debug_) {
        std::cout << "  New hit" << std::endl;
        std::cout << "    x: " << x << std::endl;
        std::cout << "    y: " << y << std::endl;
      }
      std::pair<float, float> coords;
      if (dc_ != 0 && nbrOfLayers_ > 1) {
        double i = std::ceil(std::abs(x)/dc_);
        double j = std::ceil(std::abs(y)/dc_);
        if (x < 0) {
          i = -i;
          x = (i + 0.5)*dc_;
        } else x = (i - 0.5)*dc_;
        if (y < 0) {
          j = -j;
          y = (j + 0.5)*dc_;
        } else y = (1 - 0.5)*dc_; // set x,y to middle of box
        coords = {i, j};
        if (debug_) std::cout << "    Index " << i << ", " << j << "; x: " << x << " y: " << y << std::endl;
      } else {
        coords = {fx, fy};
      }
      if (densityMap.find(coords) == densityMap.end()) {
        densityMap.emplace(coords, std::make_shared<Density>(x, y, fx, fy));
        if (debug_) std::cout << "    New density created" << std::endl;
      } else if (debug_) std::cout << "    Found density with x: " << densityMap[coords]->x << " y: " << densityMap[coords]->y << std::endl;
      densityMap[coords]->hits.push_back(hit);
      densityMap[coords]->totalEnergy += hit.getEnergy();
     
      centroid_.add(hit);
    }

    // if (debug_) finalClusters_.push_back(centroid_);
  
    // sort according to energy
    densities.reserve(densityMap.size());
    for (const auto& entry : densityMap) {
        densities.push_back(std::move(entry.second));
    }
    std::sort(densities.begin(), densities.end(), [](const std::shared_ptr<Density>& a, const std::shared_ptr<Density>& b) {
        return a->totalEnergy > b->totalEnergy;
    });
    
    if (debug_) std::cout << "Decide parents" << std::endl;

    // decide delta and followerOf
    for (int i = 0; i < densities.size(); i++) {
      densities[i]->index = i;
      if (debug_) std::cout << "  Index: " << i << "; x: " << densities[i]->x << "; y: " << densities[i]->y << "; Energy: " << densities[i]->totalEnergy << std::endl;
      // loop through all higher energy densities
      for (int j = 0; j < i; j++) {
        // double d = dist(densities[i]->x, densities[i]->y, densities[j]->x, densities[j]->y);
        float d = floatDist(densities[i]->fx, densities[i]->fy, densities[j]->fx, densities[j]->fy);
        // condition energyJ > energyI but this should be baked in as we sorted according to energy
        if (d < dm_ && d < densities[i]->fDelta) {
          densities[i]->fSecondDelta = densities[i]->fDelta;
          densities[i]->secondFollowerOf = densities[i]->followerOf;
          densities[i]->fDelta = d;
          densities[i]->followerOf = j;
          if (debug_) {
            std::cout << "    New parent, index " << j << "; delta: " << std::setprecision(20) << d << std::endl;
            std::cout << "    Updating secondary parent, index " << densities[i]->secondFollowerOf << "; delta: " << std::setprecision(20) << densities[i]->fSecondDelta << std::endl;
          }
        } else if (d < dm_ && d < densities[i]->fSecondDelta) {
          if (debug_) std::cout << "    New secondary parent, index " << j << "; delta: " << std::setprecision(20) << d << std::endl;
          densities[i]->fSecondDelta = d;
          densities[i]->secondFollowerOf = j;
        }
      }
    }
    return densities;
  }

  // std::vector<std::vector<ldmx::EcalHit>> clustering(std::vector<Density> densities, int layerTag = -1) {
  std::vector<std::vector<std::shared_ptr<Density>>> clustering(std::vector<std::shared_ptr<Density>>& densities, int layerTag = -1) {
    if (debug_) std::cout << "--- CLUSTERING ---" << std::endl;
    if (layerTag != -1 && nbrOfLayers_ > 1) {
      rhoc_ = layerRhoC_[layerTag];
      if (debug_) std::cout << "Setting rhoc on layer " << layerTag << " to " << rhoc_ << std::endl;
      if (layerTag*2-1 < radius.size()) {
        deltac_ = radius[layerTag*2-1];
        if (debug_) std::cout << "Setting deltac on layer " << layerTag << " to " << deltac_ << std::endl;
      }
    }
    // stores followers of densities at corr index
    bool energyOverload = false;
    double maxEnergy = 10000.;
    clusteringLoops_ = 0;
    double deltacMod = deltac_;
    double centroidRadius = 10.;

    std::vector<std::vector<ldmx::EcalHit>> clusters;
    std::vector<std::vector<std::shared_ptr<Density>>> densityClusters;
    std::vector<bool> mergedDensities; // index = cluster id
    mergedDensities.resize(densities.size());
    do {
      if (energyOverload) {
        deltacMod = deltacMod/1.1;
        if (debug_) std::cout << "Energy overload, new deltacmod: " << deltacMod << std::endl;
        energyOverload = false;
      }

      clusteringLoops_++;
      if (debug_) std::cout << "Clustering loop " << clusteringLoops_ << std::endl;

      int k = 0;

      std::stack<int> clusterStack;
      clusters.clear();
      clusters.reserve(densities.size());
      densityClusters.clear();
      densityClusters.reserve(densities.size());
      std::vector<double> clusterEnergies;
      clusterEnergies.reserve(densities.size());
      // stores followers of densities at corr index
      std::vector<std::vector<int>> followers;
      followers.resize(densities.size());
      
      for (int i = 0; i < densities.size(); i++) {
        if (debug_) {
          std::cout << "  Index: " << i << "; x: " << densities[i]->x << "; y: " << densities[i]->y << "; Energy: " << densities[i]->totalEnergy << std::endl;
          std::cout << "  Parent ID: " << densities[i]->followerOf << "; Delta: " << densities[i]->fDelta << std::endl;
        }

        bool isSeed;
        // if energy has been overloaded and this density belongs to cluster that was overloaded and this density is close enough to event centroid
        if (deltacMod != deltac_ && mergedDensities[densities[i]->clusterId] 
            && (densities[i]->x, densities[i]->y, centroid_.centroid().Px(), centroid_.centroid().Py()) < centroidRadius) {
          isSeed = densities[i]->totalEnergy > rhoc_ && densities[i]->fDelta > deltacMod;
        } else isSeed = densities[i]->totalEnergy > rhoc_ && densities[i]->fDelta > deltac_;
        if (debug_ && isSeed) std::cout << "  Distance to centroid: " 
                                        << dist(densities[i]->x, densities[i]->y, centroid_.centroid().Px(), centroid_.centroid().Py()) 
                                        << std::endl;
        
        bool isOutlier = densities[i]->totalEnergy < rhoc_ && densities[i]->fDelta > deltao_;

        densities[i]->clusterId = -1;
        if (isSeed) {
          if (debug_) std::cout << "  SEED, cluster id " << k << std::endl;
          densities[i]->clusterId = k;
          k++;
          clusterStack.push(i);
          clusters.push_back(densities[i]->hits);
          densityClusters.push_back( { densities[i] } );
          clusterEnergies.push_back(densities[i]->totalEnergy);
        } else if (!isOutlier) {
          if (debug_) std::cout << "  Follower" << std::endl;
          int& parentIndex = densities[i]->followerOf;
          if (parentIndex != -1) followers[parentIndex].push_back(i);
        } else if (debug_) std::cout << "  Outlier" << std::endl;
      }

      mergedDensities.clear();
      mergedDensities.resize(densities.size());

      while (clusterStack.size() > 0) {
        auto& d = densities[clusterStack.top()];
        clusterStack.pop();
        auto& cid = d->clusterId;
        for (const auto& j : followers[d->index]) { // for indices of followers of d
          auto& f = densities[j];
          // set clusterindex of follower to clusterindex of d
          f->clusterId = cid;
          clusterEnergies[cid] += f->totalEnergy;
          if (clusterEnergies[cid] > maxEnergy && clusteringLoops_ < 100) {
            mergedDensities[cid] = true;
            if (!energyOverload && clusteringLoops_ == 99) std::cout << "Merged clusters, max cluster loops hit" << std::endl;
            energyOverload = true;
            goto endwhile;
          }
          clusters[cid].insert(std::end(clusters[cid]), std::begin(f->hits), std::end(f->hits));
          densityClusters[cid].push_back(f);
          // add follower to stack, so its followers can also get correct clusterindex
          clusterStack.push(j);
        }
      }
      findMixedHits(densities, densityClusters);
      endwhile:;
    } while (energyOverload);
    // if (layerTag == 0){
    //   for (int i = 0; i < densities.size(); i++){
    //     if (densities[i].clusterId == -1) continue;
    //       for (int j = i + 1; j < densities.size(); j++){
    //         if (densities[j].clusterId == -1) continue;
    //         const auto& d = dist(densities[i].x, densities[i].y, densities[j].x, densities[j].y);
    //         // std::cout << d << std::endl;
    //         firstLayerDistances_.push_back(d);
    //       }
    //     }
    // }
    return densityClusters;
    // return clusters;    
  }

  void findMixedHits(std::vector<std::shared_ptr<Density>>& densities, std::vector<std::vector<std::shared_ptr<Density>>>& densityClusters) {
    if (debug_) std::cout << "Looking for mixed hits" << std::endl;
    for (int i = 0; i < densities.size(); i++) {
      int& parentIndex = densities[i]->followerOf;
      int& secondParentIndex = densities[i]->secondFollowerOf;
      if (densities[i]->clusterId != -1 && secondParentIndex != -1 && densities[secondParentIndex]->clusterId != -1 && densities[i]->clusterId != densities[secondParentIndex]->clusterId) {
        densities[i]->secondClusterId = densities[secondParentIndex]->clusterId;
        if (debug_) std::cout << "  Mixed density with index " << densities[i]->index << "; c1: " << densities[i]->clusterId << "; c2: " << densities[i]->secondClusterId << std::endl;
        // Calculate dist to parent vs second parent
        float dc1 = floatDist(densities[parentIndex]->fx, densities[parentIndex]->fy, densities[i]->fx, densities[i]->fy);
        float dc2 = floatDist(densities[secondParentIndex]->fx, densities[secondParentIndex]->fy, densities[i]->fx, densities[i]->fy);
        // Alt calculate dist to first cluster seed and second cluster seed -- gives slightly worse results
        // float dc1 = floatDist(densityClusters[densities[i]->clusterId][0]->fx, densityClusters[densities[i]->clusterId][0]->fy, densities[i]->x, densities[i]->y);
        // float dc2 = floatDist(densityClusters[densities[i]->secondClusterId][0]->fx, densityClusters[densities[i]->secondClusterId][0]->fy, densities[i]->x, densities[i]->y);
        if (debug_) std::cout << "    Distance to c1: " << dc1 << "; c2: " << dc2 << std::endl;
        if (dc1 + dc2 > 0)  {
          densities[i]->c1p = 1-dc1/(dc1+dc2);
          densities[i]->c2p = 1-dc2/(dc1+dc2);
        }
      }
    }
  }

  void convertToWorkingClusters(std::vector<std::vector<ldmx::EcalHit>>& clusters) {
    // Convert to workingecalclusters
    double temp;
    for (const auto& vec : clusters) {
      auto c = WorkingEcalCluster();
      for (const auto& hit : vec) {
        c.add(hit);
      }
      finalClusters_.push_back(c);
      const auto& d = dist(c.centroid().Px(), c.centroid().Py(), centroid_.centroid().Px(), centroid_.centroid().Py());
      temp += d;
      centroidDistances_.push_back(d);
    }
    avgCentroidDistance_ += temp/finalClusters_.size();
  }

  void convertDensitiesToWorkingClusters(const std::vector<std::vector<std::shared_ptr<Density>>>& densities) {
    if (debug_) std::cout << "--- CONVERTING TO WORKING CLUSTERS ---" << std::endl;
    for (const auto& cluster : densities) {
      finalClusters_.push_back(WorkingEcalCluster());
    }
    for (const auto& cluster : densities) {
      for (const auto& den : cluster) {
        if (debug_ && den->secondClusterId != -1) std::cout << "Mixed density with index " << den->index << "; cID " << den->clusterId << "; 2nd cID " << den->secondClusterId << "; c1%: " << den->c1p << "; c2%: " << den->c2p << std::endl;
        for (const auto& hit : den->hits) {
          // oh lawd
          if (den->secondClusterId != -1) {
            if (debug_) std::cout << "  Adding mixed hit with ID " << hit.getID() << "; energy: " << hit.getEnergy() << std::endl;
            finalClusters_[den->clusterId].addMixed(hit, den->c1p);
            finalClusters_[den->secondClusterId].addMixed(hit, den->c2p);
          } else {
            finalClusters_[den->clusterId].add(hit);
          }
        }
      }
    }
  }

  void cluster(std::vector<ldmx::EcalHit>& hits, double dc, double rc, double deltac, double deltao, int nbrOfLayers, bool debug) {
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
    nbrOfLayers_ = nbrOfLayers;

    if (nbrOfLayers_ < 1) nbrOfLayers_ = maxLayers_; // anything below 1 => include all layers
    else if (nbrOfLayers_ > maxLayers_) nbrOfLayers_ = maxLayers_;

    // electronSeparation(hits);
    if (nbrOfLayers_ > 1) {
      const auto layers = createLayers(hits);
      for (int i = 0; i < layers.size(); i++) {
        if (debug_) std::cout << "--- LAYER " << i << " ---" << std::endl;
        auto densities = setup(layers[i]);
        auto clusters = clustering(densities, i);
        // convertToWorkingClusters(clusters);
        convertDensitiesToWorkingClusters(clusters);
        // convertDensitiesToWorkingClusters(clustering(setup(layers[i]), i));
      }
    } else {
      auto densities = setup(hits);
      auto clusters = clustering(densities);
      // convertToWorkingClusters(clusters);
      convertDensitiesToWorkingClusters(clusters);
      // convertDensitiesToWorkingClusters(clustering(setup(hits)));
    }

  }

  double getAvgCentroidDistance() const { return avgCentroidDistance_; }

  std::vector<double> getCentroidDistances() const { return centroidDistances_; }

  std::vector<double> getFirstLayerDistances() const { return firstLayerDistances_; }

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

  int maxLayers_{17}; // layers in Ecal; a bit unsure if this is the correct number (going off layerThickness_ vector)
  int nbrOfLayers_;
  double air_{10.};
  std::vector<double> layerThickness_ = { 2., 3.5, 5.3, 5.3, 5.3, 5.3, 5.3, 5.3, 5.3, 5.3, 5.3, 10.5, 10.5, 10.5, 10.5, 10.5 };
  std::vector<double> layerRhoC_;
  std::vector<double> layerDeltaC;
  std::vector<double> radius{5.723387467629167,5.190678018534044,5.927290663506518,6.182560329200212,7.907549398117859,8.606100542857211,10.93381822596916,12.043201938160239,14.784548371508041,16.102403056546482,18.986402399412817,20.224453740305716,23.048820910305643,24.11202594672678,26.765135236851666,27.78700483852502,30.291794353801293,31.409870873194464,33.91006482486666,35.173073672355926,38.172422630271,40.880288341493205,44.696485719120005,49.23802839743545,53.789910813378675,60.87843355562641,66.32931132415688,75.78117972604727,86.04697356716805,96.90360704034346};

  double avgCentroidDistance_{0.};
  std::vector<double> centroidDistances_;
  WorkingEcalCluster centroid_;

  std::vector<double> firstLayerDistances_;

  std::vector<WorkingEcalCluster> finalClusters_;

};
}  // namespace ecal

#endif
