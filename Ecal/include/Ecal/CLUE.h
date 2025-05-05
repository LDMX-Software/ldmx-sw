/**
 * @file CLUE.h
 * @brief A version of CLUE (CMS) for clustering in ECal
 * @author Ella Viirola, Lund University
 */

#ifndef ECAL_CLUE_H_
#define ECAL_CLUE_H_

#include <math.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <stack>

#include "Ecal/Event/EcalHit.h"
#include "Ecal/WorkingEcalCluster.h"

namespace ecal {

class CLUE {
 public:
  struct Density {
    float x;
    float y;
    float z;
    double totalEnergy;
    int index;

    // index of density this density is follower of
    // set to index of spatially closest density with higher energy; -1 if seed
    int followerOf;
    // separation distance to density that this is follower of
    float delta;
    float zDelta;

    int clusterId;  // 2D cluster ID

    int layer;

    std::vector<ldmx::EcalHit> hits;

    Density() {}

    Density(float xx, float yy) : x(xx), y(yy) {
      totalEnergy = 0.;
      index = -1;
      followerOf = -1;
      delta = std::numeric_limits<float>::max();
      zDelta = std::numeric_limits<float>::max();
      clusterId = -1;
      layer = -1;
      z = 0.;
      hits = {};
    }
  };

  float dist(double x1, double y1, double x2, double y2) {
    return pow(pow(x1 - x2, 2) + pow(y1 - y2, 2), 0.5);
  }

  float floatDist(float x1, float y1, float x2, float y2) {
    return powf(powf(x1 - x2, 2) + powf(y1 - y2, 2), 0.5);
  }

  float floatDist(float x1, float y1, float z1, float x2, float y2, float z2) {
    return powf(powf(x1 - x2, 2) + powf(y1 - y2, 2) + powf(z1 - z2, 2), 0.5);
  }

  /* Old code, idea was to do electron reclustering based on first layer
     centroids' distances to each other I.e. if electrons are close together =>
     likely merged => recluster Did not quite work and I don't remember the idea
     anymore but leaving the code here for inspo */

  // void electronSeparation(std::vector<ldmx::EcalHit> hits) {
  //   std::vector<double> layerThickness =
  //   { 2., 3.5, 5.3, 5.3, 5.3, 5.3, 5.3, 5.3, 5.3, 5.3, 5.3, 10.5, 10.5, 10.5, 10.5,
  //   10.5 }; double air = 10.; std::sort(hits.begin(), hits.end(), [](const
  //   ldmx::EcalHit& a, const ldmx::EcalHit& b) {
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
  //         if (dist(firstLayerClusters[i].centroid().Px(),
  //         firstLayerClusters[i].centroid().Py(),
  //         firstLayerClusters[j].centroid().Px(),
  //         firstLayerClusters[j].centroid().Py()) < 8.) {
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
  //     if (debug_) std::cout << "  Cluster " << i << " x: " <<
  //     firstLayerClusters[i].centroid().Px() << " y: " <<
  //     firstLayerClusters[i].centroid().Py() << std::endl; for (int j = i + 1;
  //     j < firstLayerClusters.size(); j++) {
  //       if (firstLayerClusters[j].empty()) continue;
  //       auto d = dist(firstLayerClusters[i].centroid().Px(),
  //       firstLayerClusters[i].centroid().Py(),
  //       firstLayerClusters[j].centroid().Px(),
  //       firstLayerClusters[j].centroid().Py()); if (debug_) std::cout << "
  //       Dist to cluster " << j << ": " << d << std::endl;
  //     }
  //   }
  // }

  std::vector<std::vector<ldmx::EcalHit>> createLayers(
      std::vector<ldmx::EcalHit>& hits) {
    if (debug_) std::cout << "--- LAYER CREATION ---" << std::endl;
    std::vector<std::vector<ldmx::EcalHit>> layers;
    std::sort(hits.begin(), hits.end(),
              [](const ldmx::EcalHit& a, const ldmx::EcalHit& b) {
                return a.getZPos() < b.getZPos();
              });
    int layerTag = 0;
    int trueLayer = 0;
    double layerZ = hits[0].getZPos();
    double trueLayerZ = layerZ;
    double maxZ = hits[hits.size() - 1].getZPos();
    layers.push_back({});
    double layerSeparation = (maxZ - layerZ) / nbrOfLayers_;
    if (debug_)
      std::cout << "  Layer separation: " << layerSeparation << std::endl
                << "  Creating layer 0" << std::endl;

    double highestEnergy = 0.;
    int rhocFactor = 2.;
    for (const auto& hit : hits) {
      // If z of hit is in new layer, both calculated and real (we don't want to
      // split in the middle of actual ecal layer)
      if (layerTag != nbrOfLayers_ &&
          hit.getZPos() > (layerZ + layerSeparation) &&
          hit.getZPos() > trueLayerZ + layerThickness_[trueLayer] + air_) {
        layerZ = hit.getZPos();
        layers.push_back({});
        // Set seed threshold for layer to highest energy of layer / factor
        layerRhoC_.push_back(highestEnergy /
                             rhocFactor);  // TODO: decide division factor
        layerTag++;
        if (debug_)
          std::cout << "    Highest energy: " << highestEnergy << std::endl
                    << "  Creating layer " << layerTag << std::endl;
        highestEnergy = 0.;
      }
      if (hit.getZPos() > trueLayerZ + layerThickness_[trueLayer] + air_) {
        // keep track of true layers
        if (trueLayer == 0) firstLayerMaxZ_ = hit.getZPos();
        trueLayer++;
        trueLayerZ = hit.getZPos();
        if (nbrOfLayers_ < 2) return layers;
      }
      layers[layerTag].push_back(hit);
      if (hit.getEnergy() > highestEnergy) highestEnergy = hit.getEnergy();
    }
    layerRhoC_.push_back(highestEnergy / rhocFactor);
    return layers;
  }

  float roundToDecimal(float x, int num_decimal_precision_digits) {
    float power_of_10 = std::pow(10, num_decimal_precision_digits);
    return std::round(x * power_of_10) / power_of_10;
  }

  std::vector<std::shared_ptr<Density>> setup(
      const std::vector<ldmx::EcalHit>& hits) {
    std::vector<std::shared_ptr<Density>> densities;
    std::map<std::pair<float, float>, std::shared_ptr<Density>> densityMap;
    eventCentroid_ = WorkingEcalCluster();
    if (debug_)
      std::cout << "--- SETUP ---" << std::endl
                << "Building densities" << std::endl;
    for (const auto& hit : hits) {
      // collapse z dimension
      float x = roundToDecimal(hit.getXPos(), 4);
      float y = roundToDecimal(hit.getYPos(), 4);
      if (debug_) {
        std::cout << "  New hit" << std::endl;
        std::cout << "    x: " << x << std::endl;
        std::cout << "    y: " << y << std::endl;
      }
      std::pair<float, float> coords;
      if (dc_ != 0 && nbrOfLayers_ > 1) {
        // if more than one layer, divide hits into densities with side dc_
        double i = std::ceil(std::abs(x) / dc_);
        double j = std::ceil(std::abs(y) / dc_);
        if (x < 0) {
          i = -i;
          x = (i + 0.5) * dc_;
        } else
          x = (i - 0.5) * dc_;
        if (y < 0) {
          j = -j;
          y = (j + 0.5) * dc_;
        } else
          y = (1 - 0.5) * dc_;  // set x,y to middle of box
        coords = {i, j};
        if (debug_)
          std::cout << "    Index " << i << ", " << j << "; x: " << x
                    << " y: " << y << std::endl;
      } else {
        // if just one layer, have all densities with the same x,y be in same
        // density
        coords = {x, y};
      }
      if (densityMap.find(coords) == densityMap.end()) {
        densityMap.emplace(coords, std::make_shared<Density>(x, y));
        if (debug_) std::cout << "    New density created" << std::endl;
      } else if (debug_)
        std::cout << "    Found density with x: " << densityMap[coords]->x
                  << " y: " << densityMap[coords]->y << std::endl;
      densityMap[coords]->hits.push_back(hit);
      densityMap[coords]->totalEnergy += hit.getEnergy();
      densityMap[coords]->z += hit.getZPos();

      eventCentroid_.add(hit);
    }

    densities.reserve(densityMap.size());
    for (const auto& entry : densityMap) {
      densities.push_back(std::move(entry.second));
    }
    // sort according to energy
    std::sort(densities.begin(), densities.end(),
              [](const std::shared_ptr<Density>& a,
                 const std::shared_ptr<Density>& b) {
                return a->totalEnergy > b->totalEnergy;
              });

    if (debug_) std::cout << "Decide parents" << std::endl;

    // decide delta and followerOf
    for (int i = 0; i < densities.size(); i++) {
      densities[i]->index = i;
      densities[i]->z =
          densities[i]->z / densities[i]->hits.size();  // avg z position
      if (debug_)
        std::cout << "  Index: " << i << "; x: " << densities[i]->x
                  << "; y: " << densities[i]->y
                  << "; Energy: " << densities[i]->totalEnergy << std::endl;
      // loop through all higher energy densities
      for (int j = 0; j < i; j++) {
        float d = floatDist(densities[i]->x, densities[i]->y, densities[j]->x,
                            densities[j]->y);
        // condition energyJ > energyI but this should be baked in as we sorted
        // according to energy
        if (d < dm_ && d < densities[i]->delta) {
          densities[i]->delta = d;
          densities[i]->followerOf = j;
          if (debug_)
            std::cout << "    New parent, index " << j
                      << "; delta: " << std::setprecision(20) << d << std::endl;
        }
      }
    }
    return densities;
  }

  // connectingLayers marks if we're currently doing 3D clustering (i.e.
  // connecting seeds between layers) otherwise, layerTag tells us which layer
  // number we're working on
  std::vector<std::vector<ldmx::EcalHit>> clustering(
      std::vector<std::shared_ptr<Density>>& densities, bool connectingLayers,
      int layerTag = 0) {
    if (debug_) std::cout << "--- CLUSTERING ---" << std::endl;

    if (!connectingLayers && nbrOfLayers_ > 1) {
      // if doing layerwise clustering
      rhoc_ = layerRhoC_[layerTag];
      if (debug_)
        std::cout << "Setting rhoc on layer " << layerTag << " to " << rhoc_
                  << std::endl;
      if (layerTag * 2 - 1 < radius.size()) {
        deltac_ = radius[layerTag * 2 - 1];
        if (debug_)
          std::cout << "Setting deltac on layer " << layerTag << " to "
                    << deltac_ << std::endl;
      }
    } else if (connectingLayers) {
      // if currently doing 3D clustering
      // These values need to be modded; very random
      deltao_ = 200.;
      deltac_ = 100.;
      rhoc_ = 1000.;
    }

    bool energyOverload = false;
    double maxEnergy = 10000.;
    clusteringLoops_ = 0;
    double deltacMod = deltac_;
    double centroidRadius = 10.;

    // stores seeds of this layer
    std::vector<std::shared_ptr<Density>>& layerSeeds = seeds_[layerTag];

    // stores hits in cluster
    std::vector<std::vector<ldmx::EcalHit>> clusters;
    // keeps track of which densities have merged; only used if reclustering
    std::vector<bool> mergedDensities;  // index = cluster id
    mergedDensities.resize(densities.size());
    // keeps track of cluster energies
    std::vector<double> clusterEnergies;
    do {
      // while no cluster has merged
      if (energyOverload) {
        // makes delta c smaller if clusters have merged
        deltacMod = deltacMod / 1.1;
        if (debug_)
          std::cout << "Energy overload, new deltacmod: " << deltacMod
                    << std::endl;
        energyOverload = false;
      }

      clusteringLoops_++;
      if (debug_)
        std::cout << "Clustering loop " << clusteringLoops_ << std::endl;

      // cluster index
      int k = 0;

      layerSeeds.clear();
      layerSeeds.reserve(densities.size());
      clusters.clear();
      clusters.reserve(densities.size());
      clusterEnergies.clear();
      clusterEnergies.reserve(densities.size());

      std::stack<int> clusterStack;
      // stores followers of densities at corr index
      std::vector<std::vector<int>> followers;
      followers.resize(densities.size());

      // Mark as seed, follower or outlier
      for (int j = 0; j < densities.size(); j++) {
        // funky line to generalize this function for both 2D and 3D case
        int i = densities[j]->index;
        if (debug_) {
          std::cout << "  Index: " << i << "; x: " << densities[i]->x
                    << "; y: " << densities[i]->y
                    << "; Energy: " << densities[i]->totalEnergy << std::endl;
          std::cout << "  Parent ID: " << densities[i]->followerOf
                    << "; Delta: " << densities[i]->delta << std::endl;
        }

        bool isSeed;
        if (deltacMod != deltac_ && mergedDensities[densities[i]->clusterId] &&
            floatDist(densities[i]->x, densities[i]->y,
                      eventCentroid_.centroid().Px(),
                      eventCentroid_.centroid().Py()) < centroidRadius) {
          // if energy has been overloaded and this density belongs to cluster
          // that was overloaded and this density is close enough to event
          // centroid use modded delta c
          isSeed = densities[i]->totalEnergy > rhoc_ &&
                   densities[i]->delta > deltacMod;
        } else
          isSeed = densities[i]->totalEnergy > rhoc_ &&
                   densities[i]->delta > deltac_;

        if (debug_ && isSeed)
          std::cout << "  Distance to centroid: "
                    << floatDist(densities[i]->x, densities[i]->y,
                                 eventCentroid_.centroid().Px(),
                                 eventCentroid_.centroid().Py())
                    << std::endl;

        bool isOutlier =
            densities[i]->totalEnergy < rhoc_ && densities[i]->delta > deltao_;

        densities[i]->clusterId = -1;
        if (isSeed) {
          if (debug_) std::cout << "  SEED, cluster id " << k << std::endl;
          densities[i]->clusterId = k;
          k++;
          clusterStack.push(i);
          clusters.push_back(densities[i]->hits);
          clusterEnergies.push_back(densities[i]->totalEnergy);
          layerSeeds.push_back(densities[i]);
        } else if (!isOutlier) {
          if (debug_) std::cout << "  Follower" << std::endl;
          int& parentIndex = densities[i]->followerOf;
          if (parentIndex != -1) followers[parentIndex].push_back(i);
        } else if (debug_)
          std::cout << "  Outlier" << std::endl;
      }

      mergedDensities.clear();
      mergedDensities.resize(densities.size());

      // Go through all seeds and add followers, then follower's followers, etc.
      while (clusterStack.size() > 0) {
        auto& d = densities[clusterStack.top()];
        clusterStack.pop();
        auto& cid = d->clusterId;
        for (const auto& j :
             followers[d->index]) {  // for indices of followers of d
          auto& f = densities[j];
          // set clusterindex of follower to clusterindex of d
          f->clusterId = cid;
          clusterEnergies[cid] += f->totalEnergy;
          if (reclustering_ && clusterEnergies[cid] > maxEnergy &&
              deltacMod > 0.5 && clusteringLoops_ < 100) {
            // if reclustering is on and cluster energy is too high and
            // deltacmod is not too low and we haven't tried for too long
            mergedDensities[cid] = true;
            if (!energyOverload && clusteringLoops_ == 99)
              std::cout << "Merged clusters, max cluster loops hit"
                        << std::endl;
            energyOverload = true;
            if (clusteringLoops_ != 1)
              goto endwhile;  // don't break on first loop to save initial
                              // cluster number
          }
          clusters[cid].insert(std::end(clusters[cid]), std::begin(f->hits),
                               std::end(f->hits));
          // add follower to stack, so its followers can also get correct
          // clusterindex
          clusterStack.push(j);
        }
      }
      // for first clusteringloop, we want to save number of clusters before
      // reclustering
      if (clusteringLoops_ == 1 && energyOverload)
        initialClusterNbr_ = clusters.size();
    endwhile:;
    } while (energyOverload);
    // if we have more than one layer and we are not currently doing CLUE3D
    if (!connectingLayers && nbrOfLayers_ > 1) {
      // Overwrite seed densities' properties with cluster properties
      // Might be cleaner to just create new densities for cluster seeds
      for (auto& seed : layerSeeds) {
        seed->delta = std::numeric_limits<float>::max();
        seed->hits = clusters[seed->clusterId];
        seed->totalEnergy = clusterEnergies[seed->clusterId];
        seed->index = seedIndex_;
        seedIndex_++;
      }
      // Sort seeds in layer based on energy
      std::sort(layerSeeds.begin(), layerSeeds.end(),
                [](const std::shared_ptr<Density>& a,
                   const std::shared_ptr<Density>& b) {
                  return a->totalEnergy > b->totalEnergy;
                });
    }
    return clusters;
  }

  std::vector<std::shared_ptr<Density>> layerSetup() {
    if (debug_) std::cout << "--- LAYER SETUP ---" << std::endl;
    std::vector<std::shared_ptr<Density>> densities;
    layerRhoC_.clear();
    for (int layer = 0; layer < nbrOfLayers_; layer++) {
      if (debug_) std::cout << "  LAYER " << layer << std::endl;
      auto& currentLayer = seeds_[layer];
      double highestEnergy = 0.;
      for (int i = 0; i < currentLayer.size(); i++) {
        // for each seed in layer
        currentLayer[i]->layer = layer;
        if (currentLayer[i]->totalEnergy > highestEnergy)
          highestEnergy = currentLayer[i]->totalEnergy;
        if (debug_)
          std::cout << "    Density with index " << currentLayer[i]->index
                    << ", energy: " << currentLayer[i]->totalEnergy
                    << std::endl;
        int depth = 1;
        // decide delta and followerof from seeds in previous and next layer
        // do {
        // depth++;
        if (layer - depth >= 0) {
          // look at previous layer
          auto& previousLayer = seeds_[layer - depth];
          for (int j = 0; j < previousLayer.size(); j++) {
            // for each seed in previous layer
            auto d = floatDist(currentLayer[i]->x, currentLayer[i]->y,
                               previousLayer[j]->x, previousLayer[j]->y);
            auto dz = std::abs(currentLayer[i]->z - previousLayer[i]->z);
            if (debug_) {
              std::cout << "      Delta to index " << previousLayer[j]->index
                        << ": " << std::setprecision(20) << d << std::endl;
              std::cout << "      DeltaZ to index " << previousLayer[j]->index
                        << ": " << std::setprecision(20) << dz << std::endl;
            }
            if (previousLayer[j]->totalEnergy > currentLayer[i]->totalEnergy &&
                d < currentLayer[i]->delta && dz < currentLayer[i]->zDelta) {
              if (debug_) {
                std::cout << "      New parent: index "
                          << previousLayer[j]->index << " on layer "
                          << layer - depth << "; energy "
                          << previousLayer[j]->totalEnergy << std::endl;
                std::cout << "      New delta: " << std::setprecision(20) << d
                          << std::endl;
                std::cout << "      New deltaZ: " << std::setprecision(20) << dz
                          << std::endl;
              }
              currentLayer[i]->delta = d;
              currentLayer[i]->zDelta = dz;
              currentLayer[i]->followerOf = previousLayer[j]->index;
            } else if (previousLayer[j]->totalEnergy <
                       currentLayer[i]->totalEnergy)
              break;
          }
        }
        if (layer + depth < nbrOfLayers_) {
          auto& nextLayer = seeds_[layer + depth];
          for (int j = 0; j < nextLayer.size(); j++) {
            auto d = floatDist(currentLayer[i]->x, currentLayer[i]->y,
                               nextLayer[j]->x, nextLayer[j]->y);
            auto dz = std::abs(currentLayer[i]->z - nextLayer[i]->z);
            if (debug_) {
              std::cout << "      Delta to index " << nextLayer[j]->index
                        << ": " << std::setprecision(20) << d << std::endl;
              std::cout << "      DeltaZ to index " << nextLayer[j]->index
                        << ": " << std::setprecision(20) << dz << std::endl;
            }
            if (nextLayer[j]->totalEnergy > currentLayer[i]->totalEnergy &&
                d < currentLayer[i]->delta && dz < currentLayer[i]->zDelta) {
              if (debug_) {
                std::cout << "      New parent: index " << nextLayer[j]->index
                          << " on layer " << layer + depth << "; energy "
                          << nextLayer[j]->totalEnergy << std::endl;
                std::cout << "      New delta: " << std::setprecision(20) << d
                          << std::endl;
                std::cout << "      New deltaZ: " << std::setprecision(20) << dz
                          << std::endl;
              }
              currentLayer[i]->delta = d;
              currentLayer[i]->zDelta = dz;
              currentLayer[i]->followerOf = nextLayer[j]->index;
            } else if (nextLayer[j]->totalEnergy < currentLayer[i]->totalEnergy)
              break;
          }
        }
        // } while (currentLayer[i]->layerFollowerOf == -1 && (layer - depth >=
        // 0 || layer + depth < nbrOfLayers_));
        densities.push_back(currentLayer[i]);
      }
      layerRhoC_.push_back(highestEnergy / 2);
    }
    return densities;
  }

  void convertToWorkingClusters(
      std::vector<std::vector<ldmx::EcalHit>>& clusters) {
    // Convert to workingecalclusters to ensure compatibility with
    // EcalClusterProducer
    for (const auto& vec : clusters) {
      auto c = WorkingEcalCluster();
      auto fc = WorkingEcalCluster();
      for (const auto& hit : vec) {
        c.add(hit);
        // if hit is in first layer, add to first layer cluster
        if (hit.getZPos() < firstLayerMaxZ_) fc.add(hit);
      }
      finalClusters_.push_back(c);
      firstLayerCentroids_.push_back(fc);
      const auto& d =
          dist(c.centroid().Px(), c.centroid().Py(),
               eventCentroid_.centroid().Px(), eventCentroid_.centroid().Py());
      centroidDistances_.push_back(d);
    }
  }

  void cluster(std::vector<ldmx::EcalHit>& hits, double dc, double rc,
               double deltac, double deltao, int nbrOfLayers, bool reclustering,
               bool debug) {
    // cutoff distance for local density
    dc_ = dc;
    // min density to promote as seed/max density to demote as outlier
    rhoc_ = rc;
    // min separation distance for seeds
    deltac_ = deltac;
    // min separation distance for outliers
    deltao_ = deltao;
    dm_ = std::max(deltac, deltao);

    reclustering_ = reclustering;  // Recluster merged clusters or not
    debug_ = debug;
    nbrOfLayers_ = nbrOfLayers;

    if (nbrOfLayers_ < 1)
      nbrOfLayers_ = maxLayers_;  // anything below 1 => include all layers
    else if (nbrOfLayers_ > maxLayers_)
      nbrOfLayers_ = maxLayers_;

    seeds_.resize(nbrOfLayers);
    const auto layers = createLayers(hits);
    if (nbrOfLayers_ > 1) {
      for (int i = 0; i < layers.size(); i++) {
        if (debug_) std::cout << "--- LAYER " << i << " ---" << std::endl;
        auto densities = setup(layers[i]);
        auto clusters = clustering(densities, false, i);
        // convertToWorkingClusters(clusters); // uncomment for layer clustering
        // without 3D
      }
      // Below for CLUE3D, comment for just layer clustering
      auto densities = layerSetup();
      auto clusters = clustering(densities, true);
      convertToWorkingClusters(clusters);
    } else {
      auto densities = setup(hits);
      auto clusters = clustering(densities, false);
      convertToWorkingClusters(clusters);
    }
  }

  std::vector<double> getCentroidDistances() const {
    return centroidDistances_;
  }

  int getNLoops() const { return clusteringLoops_; }

  int getInitialClusterNbr() const { return initialClusterNbr_; }

  std::vector<WorkingEcalCluster> getClusters() const { return finalClusters_; }

  // First layer centroids are available for potential future combination with
  // TS
  std::vector<WorkingEcalCluster> getFirstLayerCentroids() const {
    return firstLayerCentroids_;
  }

 private:
  int clusteringLoops_;

  bool reclustering_;
  bool debug_;

  double dc_;
  double rhoc_;
  double deltac_;
  double deltao_;
  double dm_;

  int maxLayers_{
      17};  // layers in Ecal; a bit unsure if this is the correct number
  int nbrOfLayers_;
  double air_{10.};  // air between layers (a guess, but it seems to work)
  // thickness of ECal layers
  std::vector<double> layerThickness_ = {2.,   3.5,  5.3,  5.3, 5.3, 5.3,
                                         5.3,  5.3,  5.3,  5.3, 5.3, 10.5,
                                         10.5, 10.5, 10.5, 10.5};
  std::vector<double> layerRhoC_;
  std::vector<double> layerDeltaC;
  // containment radius for the different layers of the ECal
  std::vector<double> radius{
      5.723387467629167,  5.190678018534044,  5.927290663506518,
      6.182560329200212,  7.907549398117859,  8.606100542857211,
      10.93381822596916,  12.043201938160239, 14.784548371508041,
      16.102403056546482, 18.986402399412817, 20.224453740305716,
      23.048820910305643, 24.11202594672678,  26.765135236851666,
      27.78700483852502,  30.291794353801293, 31.409870873194464,
      33.91006482486666,  35.173073672355926, 38.172422630271,
      40.880288341493205, 44.696485719120005, 49.23802839743545,
      53.789910813378675, 60.87843355562641,  66.32931132415688,
      75.78117972604727,  86.04697356716805,  96.90360704034346};

  std::vector<double> centroidDistances_;
  WorkingEcalCluster eventCentroid_;

  float firstLayerMaxZ_;
  std::vector<WorkingEcalCluster> firstLayerCentroids_;

  int seedIndex_{0};
  std::vector<std::vector<std::shared_ptr<Density>>> seeds_;

  int initialClusterNbr_{-1};
  std::vector<WorkingEcalCluster> finalClusters_;
  std::vector<std::pair<double, double>> layerCentroidSeparations_;
};
}  // namespace ecal

#endif
