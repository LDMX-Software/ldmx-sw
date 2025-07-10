
#include "Ecal/CLUE.h"

namespace ecal {

float CLUE::dist(double x1, double y1, double x2, double y2) {
  return pow(pow(x1 - x2, 2) + pow(y1 - y2, 2), 0.5);
}

float CLUE::floatDist(float x1, float y1, float x2, float y2) {
  return powf(powf(x1 - x2, 2) + powf(y1 - y2, 2), 0.5);
}

float CLUE::floatDist(float x1, float y1, float z1, float x2, float y2,
                      float z2) {
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
//   std::vector<IntermediateEcalCluster> firstLayerClusters;
//   int layerTag = 0;
//   double layerZ = hits[0].getZPos();
//   for (const auto& hit : hits) {
//     if (hit.getZPos() > layerZ + layerThickness[layerTag] + air) {
//       layerTag++;
//       // if (layerTag > limit) break;
//       break;
//     }
//     firstLayers.push_back(hit);
//     firstLayerClusters.push_back(IntermediateEcalCluster(hit, layerTag));

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
//   ldmx_log(trace) << "--- ELECTRON SEPARATION ---";
//   for (int i = 0; i < firstLayerClusters.size(); i++) {
//     if (firstLayerClusters[i].empty()) continue;
//     ldmx_log(trace) << "  Cluster " << i << " x: "
//                     << firstLayerClusters[i].centroid().Px() << " y: "
//                     << firstLayerClusters[i].centroid().Py();
//     for (int j = i + 1;
//     j < firstLayerClusters.size(); j++) {
//       if (firstLayerClusters[j].empty()) continue;
//       auto d = dist(firstLayerClusters[i].centroid().Px(),
//       firstLayerClusters[i].centroid().Py(),
//       firstLayerClusters[j].centroid().Px(),
//       firstLayerClusters[j].centroid().Py());
//       ldmx_log(trace) << "Dist to cluster " << j << ": " << d;
//     }
//   }
// }

std::vector<std::vector<const ldmx::EcalHit*>> CLUE::createLayers(
    const std::vector<const ldmx::EcalHit*>& hits) {
  ldmx_log(trace) << "--- LAYER CREATION ---";
  std::vector<std::vector<const ldmx::EcalHit*>> layers;

  int layerTag = 0;
  int trueLayer = 0;
  double layerZ = hits[0]->getZPos();
  double trueLayerZ = layerZ;
  double maxZ = hits[hits.size() - 1]->getZPos();
  layers.push_back({});
  double layerSeparation = (maxZ - layerZ) / nbr_of_layers_;
  ldmx_log(trace) << "  Layer separation: " << layerSeparation;
  ldmx_log(trace) << "  Creating layer 0";

  double highestEnergy = 0.;
  int rhocFactor = 2.;
  for (const auto& hit : hits) {
    // If z of hit is in new layer, both calculated and real (we don't want to
    // split in the middle of actual ecal layer)
    if (layerTag != nbr_of_layers_ &&
        hit->getZPos() > (layerZ + layerSeparation) &&
        hit->getZPos() > trueLayerZ + layer_thickness_[trueLayer] + air_) {
      layerZ = hit->getZPos();
      layers.push_back({});
      // Set seed threshold for layer to highest energy of layer / factor
      // TODO: decide division factor
      layer_rho_c_.push_back(highestEnergy / rhocFactor);
      layerTag++;
      ldmx_log(trace) << "    Highest energy: " << highestEnergy << "\n"
                      << "  Creating layer " << layerTag;
      highestEnergy = 0.;
    }
    if (hit->getZPos() > trueLayerZ + layer_thickness_[trueLayer] + air_) {
      // keep track of true layers
      if (trueLayer == 0) first_layer_max_z_ = hit->getZPos();
      trueLayer++;
      trueLayerZ = hit->getZPos();
      if (nbr_of_layers_ < 2) return layers;
    }
    layers[layerTag].push_back(hit);
    if (hit->getEnergy() > highestEnergy) highestEnergy = hit->getEnergy();
  }
  layer_rho_c_.push_back(highestEnergy / rhocFactor);
  return layers;
}

float CLUE::roundToDecimal(float x, int num_decimal_precision_digits) {
  float power_of_10 = std::pow(10, num_decimal_precision_digits);
  return std::round(x * power_of_10) / power_of_10;
}

std::vector<std::shared_ptr<CLUE::Density>> CLUE::setup(
    const std::vector<const ldmx::EcalHit*>& hits) {
  std::vector<std::shared_ptr<Density>> densities;
  std::map<std::pair<float, float>, std::shared_ptr<Density>> densityMap;
  event_centroid_ = IntermediateCluster();
  ldmx_log(trace) << "--- SETUP ---";
  ldmx_log(trace) << "Building densities";
  for (const auto& hit : hits) {
    // collapse z dimension
    float x = roundToDecimal(hit->getXPos(), 4);
    float y = roundToDecimal(hit->getYPos(), 4);
    ldmx_log(trace) << "  New hit { x: " << x << " y: " << y << "}";
    std::pair<float, float> coords;
    if (dc_ != 0 && nbr_of_layers_ > 1) {
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
      } else                  // TODO i think this 1 should be a j
        y = (1 - 0.5) * dc_;  // set x,y to middle of box
      coords = {i, j};
      ldmx_log(trace) << "    Index " << i << ", " << j << "; x: " << x
                      << " y: " << y;
    } else {
      // if just one layer, have all densities with the same x,y be in same
      // density
      coords = {x, y};
    }
    if (densityMap.find(coords) == densityMap.end()) {
      densityMap.emplace(coords, std::make_shared<CLUE::Density>(x, y));
      ldmx_log(trace) << "    New density created";
    } else {
      ldmx_log(trace) << "    Found density with x: " << densityMap[coords]->x
                      << " y: " << densityMap[coords]->y;
    }
    densityMap[coords]->hits.push_back(hit);
    densityMap[coords]->total_energy += hit->getEnergy();
    densityMap[coords]->z += hit->getZPos();

    event_centroid_.add(hit);
  }

  densities.reserve(densityMap.size());
  for (const auto& entry : densityMap) {
    densities.push_back(std::move(entry.second));
  }
  // sort according to energy
  std::sort(densities.begin(), densities.end(),
            [](const std::shared_ptr<CLUE::Density>& a,
               const std::shared_ptr<CLUE::Density>& b) {
              return a->total_energy > b->total_energy;
            });

  ldmx_log(trace) << "Decide parents";

  // decide delta and followerOf
  for (int i = 0; i < densities.size(); i++) {
    densities[i]->index = i;
    densities[i]->z =
        densities[i]->z / densities[i]->hits.size();  // avg z position
    ldmx_log(trace) << "  Index: " << i << "; x: " << densities[i]->x
                    << "; y: " << densities[i]->y
                    << "; Energy: " << densities[i]->total_energy;
    // loop through all higher energy densities
    for (int j = 0; j < i; j++) {
      float d = floatDist(densities[i]->x, densities[i]->y, densities[j]->x,
                          densities[j]->y);
      // condition energyJ > energyI but this should be baked in as we sorted
      // according to energy
      if (d < dm_ && d < densities[i]->delta) {
        densities[i]->delta = d;
        densities[i]->follower_of = j;
        ldmx_log(trace) << "    New parent, index " << j
                        << "; delta: " << std::setprecision(20) << d;
      }
    }
  }
  return densities;
}

// connectingLayers marks if we're currently doing 3D clustering (i.e.
// connecting seeds between layers) otherwise, layerTag tells us which layer
// number we're working on
std::vector<std::vector<const ldmx::EcalHit*>> CLUE::clustering(
    std::vector<std::shared_ptr<CLUE::Density>>& densities,
    bool connectingLayers, int layerTag) {
  ldmx_log(trace) << "--- CLUSTERING ---";

  if (!connectingLayers && nbr_of_layers_ > 1) {
    // if doing layerwise clustering
    rhoc_ = layer_rho_c_[layerTag];
    ldmx_log(trace) << "Setting rhoc on layer " << layerTag << " to " << rhoc_;
    if (layerTag * 2 - 1 < radius.size()) {
      deltac_ = radius[layerTag * 2 - 1];
      ldmx_log(trace) << "Setting deltac on layer " << layerTag << " to "
                      << deltac_;
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
  clustering_loops_ = 0;
  double deltacMod = deltac_;
  double centroidRadius = 10.;

  // stores seeds of this layer
  std::vector<std::shared_ptr<Density>>& layerSeeds = seeds_[layerTag];

  // stores hits in cluster
  std::vector<std::vector<const ldmx::EcalHit*>> clusters;
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
      ldmx_log(trace) << "Energy overload, new deltacmod: " << deltacMod;
      energyOverload = false;
    }

    clustering_loops_++;
    ldmx_log(trace) << "Clustering loop " << clustering_loops_;

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
      ldmx_log(trace) << "  Index: " << i << "; x: " << densities[i]->x
                      << "; y: " << densities[i]->y
                      << "; Energy: " << densities[i]->total_energy;
      ldmx_log(trace) << "  Parent ID: " << densities[i]->follower_of
                      << "; Delta: " << densities[i]->delta;

      bool isSeed;
      if (deltacMod != deltac_ && mergedDensities[densities[i]->cluster_id] &&
          floatDist(densities[i]->x, densities[i]->y,
                    event_centroid_.centroid().x(),
                    event_centroid_.centroid().y()) < centroidRadius) {
        // if energy has been overloaded and this density belongs to cluster
        // that was overloaded and this density is close enough to event
        // centroid use modded delta c
        isSeed = densities[i]->total_energy > rhoc_ &&
                 densities[i]->delta > deltacMod;
      } else
        isSeed =
            densities[i]->total_energy > rhoc_ && densities[i]->delta > deltac_;

      if (isSeed) {
        ldmx_log(trace) << "  Distance to centroid: "
                        << floatDist(densities[i]->x, densities[i]->y,
                                     event_centroid_.centroid().Px(),
                                     event_centroid_.centroid().Py());
      }

      bool isOutlier =
          densities[i]->total_energy < rhoc_ && densities[i]->delta > deltao_;

      densities[i]->cluster_id = -1;
      if (isSeed) {
        ldmx_log(trace) << "  SEED, cluster id " << k;
        densities[i]->cluster_id = k;
        k++;
        clusterStack.push(i);
        clusters.push_back(densities[i]->hits);
        clusterEnergies.push_back(densities[i]->total_energy);
        layerSeeds.push_back(densities[i]);
      } else if (!isOutlier) {
        ldmx_log(trace) << "  Follower";
        int& parentIndex = densities[i]->follower_of;
        if (parentIndex != -1) followers[parentIndex].push_back(i);
      } else {
        ldmx_log(trace) << "  Outlier";
      }
    }

    mergedDensities.clear();
    mergedDensities.resize(densities.size());

    // Go through all seeds and add followers, then follower's followers, etc.
    while (clusterStack.size() > 0) {
      auto& d = densities[clusterStack.top()];
      clusterStack.pop();
      auto& cid = d->cluster_id;
      // for indices of followers of dp
      for (const auto& j : followers[d->index]) {
        auto& f = densities[j];
        // set clusterindex of follower to clusterindex of d
        f->cluster_id = cid;
        clusterEnergies[cid] += f->total_energy;
        if (reclustering_ && clusterEnergies[cid] > maxEnergy &&
            deltacMod > 0.5 && clustering_loops_ < 100) {
          // if reclustering is on and cluster energy is too high and
          // deltacmod is not too low and we haven't tried for too long
          mergedDensities[cid] = true;
          if (!energyOverload && clustering_loops_ == 99)
            ldmx_log(warn) << "Merging clusters, max cluster loops hit";
          energyOverload = true;
          if (clustering_loops_ != 1)
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
    if (clustering_loops_ == 1 && energyOverload)
      initial_cluster_nbr_ = clusters.size();
  endwhile:;
  } while (energyOverload);
  // if we have more than one layer and we are not currently doing CLUE3D
  if (!connectingLayers && nbr_of_layers_ > 1) {
    // Overwrite seed densities' properties with cluster properties
    // Might be cleaner to just create new densities for cluster seeds
    for (auto& seed : layerSeeds) {
      seed->delta = std::numeric_limits<float>::max();
      seed->hits = clusters[seed->cluster_id];
      seed->total_energy = clusterEnergies[seed->cluster_id];
      seed->index = seed_index_;
      seed_index_++;
    }
    // Sort seeds in layer based on energy
    std::sort(layerSeeds.begin(), layerSeeds.end(),
              [](const std::shared_ptr<Density>& a,
                 const std::shared_ptr<Density>& b) {
                return a->total_energy > b->total_energy;
              });
  }
  return clusters;
}

std::vector<std::shared_ptr<CLUE::Density>> CLUE::layerSetup() {
  ldmx_log(trace) << "--- LAYER SETUP ---";
  std::vector<std::shared_ptr<CLUE::Density>> densities;
  layer_rho_c_.clear();
  for (int layer = 0; layer < nbr_of_layers_; layer++) {
    ldmx_log(trace) << "  LAYER " << layer;
    auto& currentLayer = seeds_[layer];
    double highestEnergy = 0.;
    for (int i = 0; i < currentLayer.size(); i++) {
      // for each seed in layer
      currentLayer[i]->layer = layer;
      if (currentLayer[i]->total_energy > highestEnergy)
        highestEnergy = currentLayer[i]->total_energy;
      ldmx_log(trace) << "    Density with index " << currentLayer[i]->index
                      << ", energy: " << currentLayer[i]->total_energy;
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
          ldmx_log(trace) << "      Delta to index " << previousLayer[j]->index
                          << ": " << std::setprecision(20) << d;
          ldmx_log(trace) << "      DeltaZ to index " << previousLayer[j]->index
                          << ": " << std::setprecision(20) << dz << std::endl;
          if (previousLayer[j]->total_energy > currentLayer[i]->total_energy &&
              d < currentLayer[i]->delta && dz < currentLayer[i]->z_delta) {
            ldmx_log(trace)
                << "      New parent: index " << previousLayer[j]->index
                << " on layer " << layer - depth << "; energy "
                << previousLayer[j]->total_energy;
            ldmx_log(trace)
                << "      New delta: " << std::setprecision(20) << d;
            ldmx_log(trace)
                << "      New deltaZ: " << std::setprecision(20) << dz;
            currentLayer[i]->delta = d;
            currentLayer[i]->z_delta = dz;
            currentLayer[i]->follower_of = previousLayer[j]->index;
          } else if (previousLayer[j]->total_energy <
                     currentLayer[i]->total_energy) {
            break;
          }
        }
      }
      if (layer + depth < nbr_of_layers_) {
        auto& nextLayer = seeds_[layer + depth];
        for (int j = 0; j < nextLayer.size(); j++) {
          auto d = floatDist(currentLayer[i]->x, currentLayer[i]->y,
                             nextLayer[j]->x, nextLayer[j]->y);
          auto dz = std::abs(currentLayer[i]->z - nextLayer[i]->z);
          ldmx_log(trace) << "      Delta to index " << nextLayer[j]->index
                          << ": " << std::setprecision(20) << d;
          ldmx_log(trace) << "      DeltaZ to index " << nextLayer[j]->index
                          << ": " << std::setprecision(20) << dz;
          if (nextLayer[j]->total_energy > currentLayer[i]->total_energy &&
              d < currentLayer[i]->delta && dz < currentLayer[i]->z_delta) {
            ldmx_log(trace) << "      New parent: index " << nextLayer[j]->index
                            << " on layer " << layer + depth << "; energy "
                            << nextLayer[j]->total_energy;
            ldmx_log(trace)
                << "      New delta: " << std::setprecision(20) << d;
            ldmx_log(trace)
                << "      New deltaZ: " << std::setprecision(20) << dz;
            currentLayer[i]->delta = d;
            currentLayer[i]->z_delta = dz;
            currentLayer[i]->follower_of = nextLayer[j]->index;
          } else if (nextLayer[j]->total_energy <
                     currentLayer[i]->total_energy) {
            break;
          }
        }
      }
      // } while (currentLayer[i]->layerFollowerOf == -1 && (layer - depth >=
      // 0 || layer + depth < nbrOfLayers_));
      densities.push_back(currentLayer[i]);
    }
    layer_rho_c_.push_back(highestEnergy / 2);
  }
  return densities;
}

void CLUE::convertToIntermediateClusters(
    std::vector<std::vector<const ldmx::EcalHit*>>& clusters) {
  // Convert to workingecalclusters to ensure compatibility with
  // EcalClusterProducer
  for (const auto& vec : clusters) {
    auto c = IntermediateCluster();
    auto fc = IntermediateCluster();
    for (const auto& hit : vec) {
      c.add(hit);
      // if hit is in first layer, add to first layer cluster
      if (hit->getZPos() < first_layer_max_z_) fc.add(hit);
    }
    final_clusters_.push_back(c);
    first_layer_centroids_.push_back(fc);
    const auto& d =
        dist(c.centroid().Px(), c.centroid().Py(),
             event_centroid_.centroid().Px(), event_centroid_.centroid().Py());
    centroid_distances_.push_back(d);
  }
}

void CLUE::cluster(const std::vector<ldmx::EcalHit>& unsorted_hits, double dc,
                   double rc, double deltac, double deltao, int nbrOfLayers,
                   bool reclustering) {
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
  nbr_of_layers_ = nbrOfLayers;

  if (nbr_of_layers_ < 1)
    nbr_of_layers_ = max_layers_;  // anything below 1 => include all layers
  else if (nbr_of_layers_ > max_layers_)
    nbr_of_layers_ = max_layers_;

  // first copy *addresses* so we are only ever passing around pointers
  std::vector<const ldmx::EcalHit*> hits;
  hits.reserve(unsorted_hits.size());
  for (const auto& eh : unsorted_hits) {
    hits.push_back(&eh);
  }
  // sort hits by Z position
  std::sort(hits.begin(), hits.end(),
            [](const ldmx::EcalHit* a, const ldmx::EcalHit* b) {
              return a->getZPos() < b->getZPos();
            });

  seeds_.resize(nbrOfLayers);
  const auto layers = createLayers(hits);
  if (nbr_of_layers_ > 1) {
    for (int i = 0; i < layers.size(); i++) {
      ldmx_log(trace) << "--- LAYER " << i << " ---";
      auto densities = setup(layers[i]);
      auto clusters = clustering(densities, false, i);
      // convertToIntermediateClusters(clusters); // uncomment for layer
      // clustering without 3D
    }
    // Below for CLUE3D, comment for just layer clustering
    auto densities = layerSetup();
    auto clusters = clustering(densities, true);
    convertToIntermediateClusters(clusters);
  } else {
    auto densities = setup(hits);
    auto clusters = clustering(densities, false);
    convertToIntermediateClusters(clusters);
  }
}

}  // namespace ecal
