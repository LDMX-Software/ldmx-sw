
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

/* Old code, idea was to do electron reclustering based on first layer_
   centroids' distances to each other I.e. if electrons are close together =>
   likely merged => recluster Did not quite work and I don't remember the idea
   anymore but leaving the code here for inspo */

// void electronSeparation(std::vector<ldmx::EcalHit> hits_) {
//   std::vector<double> layerThickness =
//   { 2., 3.5, 5.3, 5.3, 5.3, 5.3, 5.3, 5.3, 5.3, 5.3, 5.3, 10.5, 10.5, 10.5, 10.5,
//   10.5 }; double air = 10.; std::sort(hits_.begin(), hits_.end(), [](const
//   ldmx::EcalHit& a, const ldmx::EcalHit& b) {
//       return a.getZPos() < b.getZPos();
//   });
//   std::vector<ldmx::EcalHit> firstLayers;
//   std::vector<IntermediateEcalCluster> firstLayerClusters;
//   int layerTag = 0;
//   double layerZ = hits_[0].getZPos();
//   for (const auto& hit : hits_) {
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
//     ldmx_log(trace) << "  Cluster " << i << " x_: "
//                     << firstLayerClusters[i].centroid().Px() << " y_: "
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
    const std::vector<const ldmx::EcalHit*>& hits_) {
  ldmx_log(trace) << "--- LAYER CREATION ---";
  std::vector<std::vector<const ldmx::EcalHit*>> layers;

  int layer_tag = 0;
  int true_layer = 0;
  double layer_z = hits_[0]->getZPos();
  double true_layer_z = layer_z;
  double max_z = hits_[hits_.size() - 1]->getZPos();
  layers.push_back({});
  double layer_separation = (max_z - layer_z) / nbr_of_layers_;
  ldmx_log(trace) << "  Layer separation: " << layer_separation;
  ldmx_log(trace) << "  Creating layer_ 0";

  double highest_energy = 0.;
  int rhoc_factor = 2.;
  for (const auto& hit : hits_) {
    // If z_ of hit is in new layer_, both calculated and real (we don't want to
    // split in the middle of actual ecal layer_)
    if (layer_tag != nbr_of_layers_ &&
        hit->getZPos() > (layer_z + layer_separation) &&
        hit->getZPos() > true_layer_z + layer_thickness_[true_layer] + air_) {
      layer_z = hit->getZPos();
      layers.push_back({});
      // Set seed threshold for layer_ to highest energy of layer_ / factor
      // TODO: decide division factor
      layer_rho_c_.push_back(highest_energy / rhoc_factor);
      layer_tag++;
      ldmx_log(trace) << "    Highest energy: " << highest_energy << "\n"
                      << "  Creating layer_ " << layer_tag;
      highest_energy = 0.;
    }
    if (hit->getZPos() > true_layer_z + layer_thickness_[true_layer] + air_) {
      // keep track of true layers
      if (true_layer == 0) first_layer_max_z_ = hit->getZPos();
      true_layer++;
      true_layer_z = hit->getZPos();
      if (nbr_of_layers_ < 2) return layers;
    }
    layers[layer_tag].push_back(hit);
    if (hit->getEnergy() > highest_energy) highest_energy = hit->getEnergy();
  }
  layer_rho_c_.push_back(highest_energy / rhoc_factor);
  return layers;
}

float CLUE::roundToDecimal(float x_, int num_decimal_precision_digits) {
  float power_of_10 = std::pow(10, num_decimal_precision_digits);
  return std::round(x_ * power_of_10) / power_of_10;
}

std::vector<std::shared_ptr<CLUE::Density>> CLUE::setup(
    const std::vector<const ldmx::EcalHit*>& hits_) {
  std::vector<std::shared_ptr<Density>> densities;
  std::map<std::pair<float, float>, std::shared_ptr<Density>> density_map;
  event_centroid_ = IntermediateCluster();
  ldmx_log(trace) << "--- SETUP ---";
  ldmx_log(trace) << "Building densities";
  for (const auto& hit : hits_) {
    // collapse z_ dimension
    float x_ = roundToDecimal(hit->getXPos(), 4);
    float y_ = roundToDecimal(hit->getYPos(), 4);
    ldmx_log(trace) << "  New hit { x_: " << x_ << " y_: " << y_ << "}";
    std::pair<float, float> coords;
    if (dc_ != 0 && nbr_of_layers_ > 1) {
      // if more than one layer_, divide hits_ into densities with side dc_
      double i = std::ceil(std::abs(x_) / dc_);
      double j = std::ceil(std::abs(y_) / dc_);
      if (x_ < 0) {
        i = -i;
        x_ = (i + 0.5) * dc_;
      } else
        x_ = (i - 0.5) * dc_;
      if (y_ < 0) {
        j = -j;
        y_ = (j + 0.5) * dc_;
      } else                   // TODO i think this 1 should be a j
        y_ = (1 - 0.5) * dc_;  // set x_,y_ to middle of box
      coords = {i, j};
      ldmx_log(trace) << "    Index " << i << ", " << j << "; x_: " << x_
                      << " y_: " << y_;
    } else {
      // if just one layer_, have all densities with the same x_,y_ be in same
      // density
      coords = {x_, y_};
    }
    if (density_map.find(coords) == density_map.end()) {
      density_map.emplace(coords, std::make_shared<CLUE::Density>(x_, y_));
      ldmx_log(trace) << "    New density created";
    } else {
      ldmx_log(trace) << "    Found density with x_: "
                      << density_map[coords]->x_
                      << " y_: " << density_map[coords]->y_;
    }
    density_map[coords]->hits_.push_back(hit);
    density_map[coords]->total_energy_ += hit->getEnergy();
    density_map[coords]->z_ += hit->getZPos();

    event_centroid_.add(hit);
  }

  densities.reserve(density_map.size());
  for (const auto& entry : density_map) {
    densities.push_back(std::move(entry.second));
  }
  // sort according to energy
  std::sort(densities.begin(), densities.end(),
            [](const std::shared_ptr<CLUE::Density>& a,
               const std::shared_ptr<CLUE::Density>& b) {
              return a->total_energy_ > b->total_energy_;
            });

  ldmx_log(trace) << "Decide parents";

  // decide delta_ and followerOf
  for (int i = 0; i < densities.size(); i++) {
    densities[i]->index_ = i;
    densities[i]->z_ =
        densities[i]->z_ / densities[i]->hits_.size();  // avg z_ position
    ldmx_log(trace) << "  Index: " << i << "; x_: " << densities[i]->x_
                    << "; y_: " << densities[i]->y_
                    << "; Energy: " << densities[i]->total_energy_;
    // loop through all higher energy densities
    for (int j = 0; j < i; j++) {
      float d = floatDist(densities[i]->x_, densities[i]->y_, densities[j]->x_,
                          densities[j]->y_);
      // condition energyJ > energyI but this should be baked in as we sorted
      // according to energy
      if (d < dm_ && d < densities[i]->delta_) {
        densities[i]->delta_ = d;
        densities[i]->follower_of_ = j;
        ldmx_log(trace) << "    New parent, index_" << j
                        << "; delta_: " << std::setprecision(20) << d;
      }
    }
  }
  return densities;
}

// connectingLayers marks if we're currently doing 3D clustering (i.e.
// connecting seeds between layers) otherwise, layerTag tells us which layer_
// number we're working on
std::vector<std::vector<const ldmx::EcalHit*>> CLUE::clustering(
    std::vector<std::shared_ptr<CLUE::Density>>& densities,
    bool connectingLayers, int layerTag) {
  ldmx_log(trace) << "--- CLUSTERING ---";

  if (!connectingLayers && nbr_of_layers_ > 1) {
    // if doing layerwise clustering
    rhoc_ = layer_rho_c_[layerTag];
    ldmx_log(trace) << "Setting rhoc on layer_ " << layerTag << " to " << rhoc_;
    if (layerTag * 2 - 1 < radius_.size()) {
      deltac_ = radius_[layerTag * 2 - 1];
      ldmx_log(trace) << "Setting delta_c on layer_ " << layerTag << " to "
                      << deltac_;
    }
  } else if (connectingLayers) {
    // if currently doing 3D clustering
    // These values need to be modded; very random
    deltao_ = 200.;
    deltac_ = 100.;
    rhoc_ = 1000.;
  }

  bool energy_overload = false;
  double max_energy = 10000.;
  clustering_loops_ = 0;
  double delta_c_mod = deltac_;
  double centroid_radius = 10.;

  // stores seeds of this layer_
  std::vector<std::shared_ptr<Density>>& layer_seeds = seeds_[layerTag];

  // stores hits_ in cluster
  std::vector<std::vector<const ldmx::EcalHit*>> clusters;
  // keeps track of which densities have merged; only used if reclustering
  std::vector<bool> merged_densities;  // index_= cluster id
  merged_densities.resize(densities.size());
  // keeps track of cluster energies
  std::vector<double> cluster_energies;
  do {
    // while no cluster has merged
    if (energy_overload) {
      // makes delta_ c smaller if clusters have merged
      delta_c_mod = delta_c_mod / 1.1;
      ldmx_log(trace) << "Energy overload, new delta_cmod: " << delta_c_mod;
      energy_overload = false;
    }

    clustering_loops_++;
    ldmx_log(trace) << "Clustering loop " << clustering_loops_;

    // cluster index_
    int k = 0;

    layer_seeds.clear();
    layer_seeds.reserve(densities.size());
    clusters.clear();
    clusters.reserve(densities.size());
    cluster_energies.clear();
    cluster_energies.reserve(densities.size());

    std::stack<int> cluster_stack;
    // stores followers of densities at corr index_
    std::vector<std::vector<int>> followers;
    followers.resize(densities.size());

    // Mark as seed, follower or outlier
    for (int j = 0; j < densities.size(); j++) {
      // funky line to generalize this function for both 2D and 3D case
      int i = densities[j]->index_;
      ldmx_log(trace) << "  Index: " << i << "; x_: " << densities[i]->x_
                      << "; y_: " << densities[i]->y_
                      << "; Energy: " << densities[i]->total_energy_;
      ldmx_log(trace) << "  Parent ID: " << densities[i]->follower_of_
                      << "; Delta: " << densities[i]->delta_;

      bool is_seed;
      if (delta_c_mod != deltac_ &&
          merged_densities[densities[i]->cluster_id_] &&
          floatDist(densities[i]->x_, densities[i]->y_,
                    event_centroid_.centroid().x(),
                    event_centroid_.centroid().y()) < centroid_radius) {
        // if energy has been overloaded and this density belongs to cluster
        // that was overloaded and this density is close enough to event
        // centroid use modded delta_ c
        is_seed = densities[i]->total_energy_ > rhoc_ &&
                  densities[i]->delta_ > delta_c_mod;
      } else
        is_seed = densities[i]->total_energy_ > rhoc_ &&
                  densities[i]->delta_ > deltac_;

      if (is_seed) {
        ldmx_log(trace) << "  Distance to centroid: "
                        << floatDist(densities[i]->x_, densities[i]->y_,
                                     event_centroid_.centroid().Px(),
                                     event_centroid_.centroid().Py());
      }

      bool is_outlier =
          densities[i]->total_energy_ < rhoc_ && densities[i]->delta_ > deltao_;

      densities[i]->cluster_id_ = -1;
      if (is_seed) {
        ldmx_log(trace) << "  SEED, cluster id " << k;
        densities[i]->cluster_id_ = k;
        k++;
        cluster_stack.push(i);
        clusters.push_back(densities[i]->hits_);
        cluster_energies.push_back(densities[i]->total_energy_);
        layer_seeds.push_back(densities[i]);
      } else if (!is_outlier) {
        ldmx_log(trace) << "  Follower";
        int& parent_index_ = densities[i]->follower_of_;
        if (parent_index_ != -1) followers[parent_index_].push_back(i);
      } else {
        ldmx_log(trace) << "  Outlier";
      }
    }

    merged_densities.clear();
    merged_densities.resize(densities.size());

    // Go through all seeds and add followers, then follower's followers, etc.
    while (cluster_stack.size() > 0) {
      auto& d = densities[cluster_stack.top()];
      cluster_stack.pop();
      auto& cid = d->cluster_id_;
      // for indices of followers of dp
      for (const auto& j : followers[d->index_]) {
        auto& f = densities[j];
        // set clusterindex_of follower to clusterindex_of d
        f->cluster_id_ = cid;
        cluster_energies[cid] += f->total_energy_;
        if (reclustering_ && cluster_energies[cid] > max_energy &&
            delta_c_mod > 0.5 && clustering_loops_ < 100) {
          // if reclustering is on and cluster energy is too high and
          // delta_cmod is not too low and we haven't tried for too long
          merged_densities[cid] = true;
          if (!energy_overload && clustering_loops_ == 99)
            ldmx_log(warn) << "Merging clusters, max cluster loops hit";
          energy_overload = true;
          if (clustering_loops_ != 1)
            goto endwhile;  // don't break on first loop to save initial
                            // cluster number
        }
        clusters[cid].insert(std::end(clusters[cid]), std::begin(f->hits_),
                             std::end(f->hits_));
        // add follower to stack, so its followers can also get correct
        // clusterindex
        cluster_stack.push(j);
      }
    }
    // for first clusteringloop, we want to save number of clusters before
    // reclustering
    if (clustering_loops_ == 1 && energy_overload)
      initial_cluster_nbr_ = clusters.size();
  endwhile:;
  } while (energy_overload);
  // if we have more than one layer_ and we are not currently doing CLUE3D
  if (!connectingLayers && nbr_of_layers_ > 1) {
    // Overwrite seed densities' properties with cluster properties
    // Might be cleaner to just create new densities for cluster seeds
    for (auto& seed : layer_seeds) {
      seed->delta_ = std::numeric_limits<float>::max();
      seed->hits_ = clusters[seed->cluster_id_];
      seed->total_energy_ = cluster_energies[seed->cluster_id_];
      seed->index_ = seed_index_;
      seed_index_++;
    }
    // Sort seeds in layer_ based on energy
    std::sort(layer_seeds.begin(), layer_seeds.end(),
              [](const std::shared_ptr<Density>& a,
                 const std::shared_ptr<Density>& b) {
                return a->total_energy_ > b->total_energy_;
              });
  }
  return clusters;
}

std::vector<std::shared_ptr<CLUE::Density>> CLUE::layerSetup() {
  ldmx_log(trace) << "--- LAYER SETUP ---";
  std::vector<std::shared_ptr<CLUE::Density>> densities;
  layer_rho_c_.clear();
  for (int layer_ = 0; layer_ < nbr_of_layers_; layer_++) {
    ldmx_log(trace) << "  LAYER " << layer_;
    auto& current_layer = seeds_[layer_];
    double highest_energy = 0.;
    for (int i = 0; i < current_layer.size(); i++) {
      // for each seed in layer_
      current_layer[i]->layer_ = layer_;
      if (current_layer[i]->total_energy_ > highest_energy)
        highest_energy = current_layer[i]->total_energy_;
      ldmx_log(trace) << "    Density with index_" << current_layer[i]->index_
                      << ", energy: " << current_layer[i]->total_energy_;
      int depth = 1;
      // decide delta_ and followerof from seeds in previous and next layer_
      // do {
      // depth++;
      if (layer_ - depth >= 0) {
        // look at previous layer_
        auto& previous_layer = seeds_[layer_ - depth];
        for (int j = 0; j < previous_layer.size(); j++) {
          // for each seed in previous layer_
          auto d = floatDist(current_layer[i]->x_, current_layer[i]->y_,
                             previous_layer[j]->x_, previous_layer[j]->y_);
          auto dz = std::abs(current_layer[i]->z_ - previous_layer[i]->z_);
          ldmx_log(trace) << "      Delta to index_"
                          << previous_layer[j]->index_ << ": "
                          << std::setprecision(20) << d;
          ldmx_log(trace) << "      DeltaZ to index_"
                          << previous_layer[j]->index_ << ": "
                          << std::setprecision(20) << dz << std::endl;
          if (previous_layer[j]->total_energy_ >
                  current_layer[i]->total_energy_ &&
              d < current_layer[i]->delta_ && dz < current_layer[i]->z_delta_) {
            ldmx_log(trace)
                << "      New parent: index_" << previous_layer[j]->index_
                << " on layer_ " << layer_ - depth << "; energy "
                << previous_layer[j]->total_energy_;
            ldmx_log(trace)
                << "      New delta_: " << std::setprecision(20) << d;
            ldmx_log(trace)
                << "      New delta_Z: " << std::setprecision(20) << dz;
            current_layer[i]->delta_ = d;
            current_layer[i]->z_delta_ = dz;
            current_layer[i]->follower_of_ = previous_layer[j]->index_;
          } else if (previous_layer[j]->total_energy_ <
                     current_layer[i]->total_energy_) {
            break;
          }
        }
      }
      if (layer_ + depth < nbr_of_layers_) {
        auto& next_layer = seeds_[layer_ + depth];
        for (int j = 0; j < next_layer.size(); j++) {
          auto d = floatDist(current_layer[i]->x_, current_layer[i]->y_,
                             next_layer[j]->x_, next_layer[j]->y_);
          auto dz = std::abs(current_layer[i]->z_ - next_layer[i]->z_);
          ldmx_log(trace) << "      Delta to index_" << next_layer[j]->index_
                          << ": " << std::setprecision(20) << d;
          ldmx_log(trace) << "      DeltaZ to index_" << next_layer[j]->index_
                          << ": " << std::setprecision(20) << dz;
          if (next_layer[j]->total_energy_ > current_layer[i]->total_energy_ &&
              d < current_layer[i]->delta_ && dz < current_layer[i]->z_delta_) {
            ldmx_log(trace)
                << "      New parent: index_" << next_layer[j]->index_
                << " on layer_ " << layer_ + depth << "; energy "
                << next_layer[j]->total_energy_;
            ldmx_log(trace)
                << "      New delta_: " << std::setprecision(20) << d;
            ldmx_log(trace)
                << "      New delta_Z: " << std::setprecision(20) << dz;
            current_layer[i]->delta_ = d;
            current_layer[i]->z_delta_ = dz;
            current_layer[i]->follower_of_ = next_layer[j]->index_;
          } else if (next_layer[j]->total_energy_ <
                     current_layer[i]->total_energy_) {
            break;
          }
        }
      }
      // } while (currentLayer[i]->layerFollowerOf == -1 && (layer_ - depth >=
      // 0 || layer_ + depth < nbrOfLayers_));
      densities.push_back(current_layer[i]);
    }
    layer_rho_c_.push_back(highest_energy / 2);
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
      // if hit is in first layer_, add to first layer_ cluster
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
                   double rc, double delta_c, double delta_o, int nbrOfLayers,
                   bool reclustering) {
  // cutoff distance for local density
  dc_ = dc;
  // min density to promote as seed/max density to demote as outlier
  rhoc_ = rc;
  // min separation distance for seeds
  deltac_ = delta_c;
  // min separation distance for outliers
  deltao_ = delta_o;
  dm_ = std::max(delta_c, delta_o);

  reclustering_ = reclustering;  // Recluster merged clusters or not
  nbr_of_layers_ = nbrOfLayers;

  if (nbr_of_layers_ < 1)
    nbr_of_layers_ = max_layers_;  // anything below 1 => include all layers
  else if (nbr_of_layers_ > max_layers_)
    nbr_of_layers_ = max_layers_;

  // first copy *addresses* so we are only ever passing around pointers
  std::vector<const ldmx::EcalHit*> hits_;
  hits_.reserve(unsorted_hits.size());
  for (const auto& eh : unsorted_hits) {
    hits_.push_back(&eh);
  }
  // sort hits_ by Z position
  std::sort(hits_.begin(), hits_.end(),
            [](const ldmx::EcalHit* a, const ldmx::EcalHit* b) {
              return a->getZPos() < b->getZPos();
            });

  seeds_.resize(nbrOfLayers);
  const auto layers = createLayers(hits_);
  if (nbr_of_layers_ > 1) {
    for (int i = 0; i < layers.size(); i++) {
      ldmx_log(trace) << "--- LAYER " << i << " ---";
      auto densities = setup(layers[i]);
      auto clusters = clustering(densities, false, i);
      // convertToIntermediateClusters(clusters); // uncomment for layer_
      // clustering without 3D
    }
    // Below for CLUE3D, comment for just layer_ clustering
    auto densities = layerSetup();
    auto clusters = clustering(densities, true);
    convertToIntermediateClusters(clusters);
  } else {
    auto densities = setup(hits_);
    auto clusters = clustering(densities, false);
    convertToIntermediateClusters(clusters);
  }
}

}  // namespace ecal
