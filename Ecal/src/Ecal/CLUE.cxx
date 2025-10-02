
#include "Ecal/CLUE.h"

#include <cmath>

namespace ecal {

/**
 * Euclidean distance between two points
 * Using x*x and std::sqrt since they are more specialized and faster than
 * std::pow
 * @tparam T floating point type
 */
template <typename T>
T CLUE::dist(T x1, T y1, T x2, T y2) {
  auto delta_x = x1 - x2;
  auto delta_y = y1 - y2;
  auto r_square = delta_x * delta_x + delta_y * delta_y;
  return std::sqrt(r_square);
}

// 3D version, overloaded
template <typename T>
T CLUE::dist(T x1, T y1, T z1, T x2, T y2, T z2) {
  auto delta_x = x1 - x2;
  auto delta_y = y1 - y2;
  auto delta_z = z1 - z2;
  auto r_square = delta_x * delta_x + delta_y * delta_y + delta_z * delta_z;
  return std::sqrt(r_square);
}

/* Old code, idea was to do electron reclustering based on first layer
   centroids' distances to each other I.e. if electrons are close together =>
   likely merged => recluster Did not quite work and I don't remember the idea
   anymore but leaving the code here for inspo */

void CLUE::electronSeparation(std::vector<ldmx::EcalHit> hits) {
  std::vector<double> layer_thickness = {2.,   3.5,  5.3,  5.3, 5.3, 5.3,
                                        5.3,  5.3,  5.3,  5.3, 5.3, 10.5,
                                        10.5, 10.5, 10.5, 10.5};
  double air = 10.;
  // sort hits in z
  std::sort(hits.begin(), hits.end(),
            [](const ldmx::EcalHit& a, const ldmx::EcalHit& b) {
              return a.getZPos() < b.getZPos();
            });

  std::vector<ldmx::EcalHit> first_layers;
  std::vector<IntermediateCluster> first_layer_clusters;
  int layer_tag = 0;
  double layer_z = hits[0].getZPos();
  for (const auto& hit : hits) {
    if (hit.getZPos() > layer_z + layer_thickness[layer_tag] + air) {
      layer_tag++;
      // if (layerTag > limit) break;
      break;
    }
    first_layers.push_back(hit);
    first_layer_clusters.push_back(IntermediateCluster(hit, layer_tag));
  }
  bool merge = false;
  do {
    merge = false;
    for (int i = 0; i < first_layer_clusters.size(); i++) {
      if (first_layer_clusters[i].empty()) continue;
      // if (firstLayerClusters[i].centroid().E() >= seedThreshold_) {
      for (int j = i + 1; j < first_layer_clusters.size(); j++) {
        if (first_layer_clusters[j].empty()) continue;
        if (dist(first_layer_clusters[i].centroid().Px(),
                 first_layer_clusters[i].centroid().Py(),
                 first_layer_clusters[j].centroid().Px(),
                 first_layer_clusters[j].centroid().Py()) < 8.) {
          first_layer_clusters[i].add(first_layer_clusters[j]);
          first_layer_clusters[j].clear();
          merge = true;
        }
      }
      // } else break;
    }
  } while (merge);
  ldmx_log(trace) << "--- ELECTRON SEPARATION ---";
  for (int i = 0; i < first_layer_clusters.size(); i++) {
    if (first_layer_clusters[i].empty()) continue;
    ldmx_log(trace) << "  Cluster " << i
                    << " x: " << first_layer_clusters[i].centroid().Px()
                    << " y: " << first_layer_clusters[i].centroid().Py();
    for (int j = i + 1; j < first_layer_clusters.size(); j++) {
      if (first_layer_clusters[j].empty()) continue;
      auto d = dist(first_layer_clusters[i].centroid().Px(),
                    first_layer_clusters[i].centroid().Py(),
                    first_layer_clusters[j].centroid().Px(),
                    first_layer_clusters[j].centroid().Py());
      ldmx_log(trace) << "Dist to cluster " << j << ": " << d;
    }
  }
}

// Function to create layers from hits based on their layer number
std::vector<std::vector<const ldmx::EcalHit*>> CLUE::createLayers(
    const std::vector<const ldmx::EcalHit*>& hits) {
  ldmx_log(trace) << "--- LAYER CREATION ---";
  ldmx_log(trace) << "Number of layers: " << nbr_of_layers_;

  // vector of layers, each layer having a vector of hits
  // initialize with nbr_of_layers_ empty vectors
  std::vector<std::vector<const ldmx::EcalHit*>> hits_per_layer(nbr_of_layers_);

  // Track highest energy per layer for proper rho_c calculation
  std::vector<double> layer_max_energies(nbr_of_layers_, 0.0);

  // Clear any existing layer_rho_c_ values
  layer_rho_c_.clear();

  // This is rather ad-hoc, but this way it's still tunable via rhoc_ parameter
  double rhoc_factor = rhoc_ / 250.;

  for (const auto& hit : hits) {
    ldmx::EcalID ecal_id(hit->getID());
    // layer number from EcalID, starting at 0
    int layer = ecal_id.layer();

    if (layer >= nbr_of_layers_) {
      ldmx_log(trace) << "Skipping hit in layer " << layer
                      << " (beyond nbr_of_layers_ = " << nbr_of_layers_ << ")";
      continue;
    }

    // Track the highest energy in this specific layer
    if (hit->getEnergy() > layer_max_energies[layer]) {
      layer_max_energies[layer] = hit->getEnergy();
    }

    ldmx_log(trace) << "  Adding hit with energy " << hit->getEnergy()
                    << " to layer " << layer;
    hits_per_layer[layer].push_back(hit);
  }  // end of loop over hits

  // Calculate rhoc for each layer based on that layer's maximum energy
  for (int i = 0; i < nbr_of_layers_; i++) {
    double rho_c = layer_max_energies[i] / rhoc_factor;
    layer_rho_c_.push_back(rho_c);
    ldmx_log(trace) << "Layer " << i << " has " << hits_per_layer[i].size()
                    << " hits, max energy " << layer_max_energies[i]
                    << ", rho_c " << rho_c;
  }

  ldmx_log(trace) << "Created " << hits_per_layer.size() << " layers";
  return hits_per_layer;
}  // end of createLayers

float CLUE::roundToDecimal(float x_, int num_decimal_precision_digits) {
  float power_of_10 = std::pow(10, num_decimal_precision_digits);
  return std::round(x_ * power_of_10) / power_of_10;
}

std::vector<std::shared_ptr<CLUE::Density>> CLUE::setup(
    const std::vector<const ldmx::EcalHit*>& hits) {
  std::vector<std::shared_ptr<Density>> densities;
  std::map<std::pair<float, float>, std::shared_ptr<Density>> density_map;
  event_centroid_ = IntermediateCluster();
  ldmx_log(trace) << "--- SETUP ---";
  ldmx_log(trace) << "Building densities";
  for (const auto& hit : hits) {
    // collapse z dimension
    float x = roundToDecimal(hit->getXPos(), 4);
    float y = roundToDecimal(hit->getYPos(), 4);
    float z = roundToDecimal(hit->getZPos(), 4);
    ldmx_log(trace) << "  New hit { x: " << x << " y: " << y << "}"
                    << " (and z: " << z << ")";
    std::pair<float, float> coords;
    if (dc_ != 0 && nbr_of_layers_ > 1) {
      // if more than one layer, divide hit into densities with side dc
      double i = std::ceil(std::abs(x) / dc_);
      double j = std::ceil(std::abs(y) / dc_);
      if (x < 0) {
        i = -i;
        x = (i + 0.5) * dc_;
      } else {
        x = (i - 0.5) * dc_;
      }
      if (y < 0) {
        j = -j;
        y = (j + 0.5) * dc_;
      } else {
        y = (j - 0.5) * dc_;
      }
      coords = {i, j};
      ldmx_log(trace) << "    Index " << i << ", " << j << "; x: " << x
                      << " y: " << y;
    } else {
      // if just one layer, have all densities with the same x,y be in same
      // density
      coords = {x, y};
    }

    if (density_map.find(coords) == density_map.end()) {
      density_map.emplace(coords, std::make_shared<CLUE::Density>(x, y));
      ldmx_log(trace) << "  * New density created";
    } else {
      ldmx_log(trace) << "  --> Found density with x: "
                      << density_map[coords]->x_
                      << " y: " << density_map[coords]->y_;
    }
    density_map[coords]->hits_.push_back(hit);
    density_map[coords]->total_energy_ += hit->getEnergy();
    density_map[coords]->z_ += hit->getZPos();

    event_centroid_.add(hit);
  }  // end of loop over hits

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

  // decide delta_ and follower_of_
  for (int i = 0; i < densities.size(); i++) {
    densities[i]->index_ = i;
    // avg z position
    densities[i]->z_ = densities[i]->z_ / densities[i]->hits_.size();
    ldmx_log(trace) << "  Index: " << i << "; x: " << densities[i]->x_
                    << "; y: " << densities[i]->y_
                    << "; Energy: " << densities[i]->total_energy_;
    // loop through all higher energy densities
    for (int j = 0; j < i; j++) {
      float distance_2d = dist(densities[i]->x_, densities[i]->y_,
                               densities[j]->x_, densities[j]->y_);
      // condition energyJ > energyI but this should be baked in as we sorted
      // according to energy
      if ((distance_2d < dm_) && (distance_2d < densities[i]->delta_)) {
        densities[i]->delta_ = distance_2d;
        densities[i]->follower_of_ = j;
        ldmx_log(trace) << "    New parent, index " << j
                        << "; delta_2d: " << std::setprecision(4)
                        << distance_2d;
      }
    }
  }
  return densities;
}  // end of setup

// connecting_layers marks if we're currently doing 3D clustering (i.e.
// connecting seeds between layers) otherwise, layer_index tells us which layer
// number we're working on
std::vector<std::vector<const ldmx::EcalHit*>> CLUE::clustering(
    std::vector<std::shared_ptr<CLUE::Density>>& densities,
    bool connecting_layers, int layer_index) {
  ldmx_log(trace) << "--- CLUSTERING ---";
  ldmx_log(trace) << "Number of densities: " << densities.size()
                  << "; connecting_layers: " << connecting_layers
                  << "; layer_index: " << layer_index;
  if (!connecting_layers && nbr_of_layers_ > 1) {
    // if layerwise clustering override rhoc_ and deltac_ with per-layer values
    rhoc_ = layer_rho_c_[layer_index];
    ldmx_log(trace) << "Setting rho_c on layer " << layer_index << " to "
                    << rhoc_;
    if (layer_index * 2 - 1 < radius_.size()) {
      deltac_ = radius_[layer_index * 2 - 1];
      ldmx_log(trace) << "Setting delta_c on layer " << layer_index << " to "
                      << deltac_;
    }
  } else if (connecting_layers) {
    // if doing 3D clustering, override deltac_ and rhoc_ with other values
    // NOT IMPLEMENTED YET
    // if currently doing 3D clustering
    // IF 3D clustering
    // deltao_ = 200.;
    // deltac_ = 100.;
    // rhoc_ = 1000.;
  }

  bool energy_overload = false;
  double max_energy = 10000.;
  clustering_loops_ = 0;
  double delta_c_mod = deltac_;
  double centroid_radius = 10.;

  // stores seeds of this layer
  std::vector<std::shared_ptr<Density>>& layer_seeds = seeds_[layer_index];

  // stores hits in cluster
  std::vector<std::vector<const ldmx::EcalHit*>> clusters;
  // keeps track of which densities have merged; only used if reclustering
  std::vector<bool> merged_densities;  // index_= cluster id
  merged_densities.resize(densities.size());
  // keeps track of cluster energies
  std::vector<double> cluster_energies;
  do {
    // while no cluster has merged
    if (energy_overload) {
      // makes delta_c smaller if clusters have merged
      delta_c_mod = delta_c_mod / 1.1;
      ldmx_log(trace) << "Energy overload, new delta_cmod: " << delta_c_mod;
      energy_overload = false;
    }

    clustering_loops_++;
    ldmx_log(trace) << "Clustering loop " << clustering_loops_;

    // cluster index
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

    // Mark as seed, follower, or outlier
    for (auto& density : densities) {
      // funky line to generalize this function for both 2D and 3D case
      ldmx_log(trace) << "   Index: " << density->index_
                      << "; x: " << density->x_ << "; y: " << density->y_
                      << "; Energy: " << density->total_energy_
                      << "    Parent ID: " << density->follower_of_
                      << "; Delta: " << density->delta_;

      bool is_seed;
      if (delta_c_mod != deltac_ && merged_densities[density->cluster_id_] &&
          dist(density->x_, density->y_, event_centroid_.centroid().x(),
               event_centroid_.centroid().y()) < centroid_radius) {
        // if energy has been overloaded and this density belongs to cluster
        // that was overloaded and this density is close enough to event
        // centroid use modded delta c
        is_seed =
            density->total_energy_ > rhoc_ && density->delta_ > delta_c_mod;
      } else {
        is_seed = density->total_energy_ > rhoc_ && density->delta_ > deltac_;
        if (is_seed) {
          ldmx_log(trace) << "  Distance to event centroid: "
                          << dist(density->x_, density->y_,
                                  event_centroid_.centroid().x(),
                                  event_centroid_.centroid().y());
        }
      }
      bool is_outlier =
          (density->total_energy_ < rhoc_) && (density->delta_ > deltao_);
      density->cluster_id_ = -1;
      if (is_seed) {
        ldmx_log(trace) << "      This is a Seed";
        ldmx_log(trace) << "      Distance to centroid: "
                        << dist(density->x_, density->y_,
                                event_centroid_.centroid().x(),
                                event_centroid_.centroid().y())
                        << "; with delta " << density->delta_;
        ldmx_log(trace) << "      Setting cluster ID to " << k;
        density->cluster_id_ = k;
        k++;
        // get the index of the seed density
        cluster_stack.push(density->index_);
        clusters.push_back(density->hits_);
        cluster_energies.push_back(density->total_energy_);
        layer_seeds.push_back(density);
      } else if (!is_outlier) {
        ldmx_log(trace) << "      This is a Follower";
        int& parent_index = density->follower_of_;
        if (parent_index != -1)
          followers[parent_index].push_back(density->index_);
        else
          ldmx_log(error)
              << "  Somehow found a follower with parent index -1: id = "
              << density->index_;
      } else {
        ldmx_log(trace) << "      This is an Outlier";
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
      for (const auto follower_index : followers[d->index_]) {
        auto& follower = densities[follower_index];
        // Set cluster index of the follower to the cluster index of `d`
        follower->cluster_id_ = cid;
        cluster_energies[cid] += follower->total_energy_;

        if (reclustering_ && cluster_energies[cid] > max_energy &&
            delta_c_mod > 0.5 && clustering_loops_ < 100) {
          // If reclustering is on and cluster energy is too high,
          // delta_c_mod is not too low, and we haven't tried for too long
          merged_densities[cid] = true;
          if (!energy_overload && clustering_loops_ == 99) {
            ldmx_log(warn) << "Merging clusters, max cluster loops hit";
          }
          energy_overload = true;
          if (clustering_loops_ != 1) {
            goto endwhile;  // Don't break on the first loop to save the initial
                            // cluster number
          }
        }

        clusters[cid].insert(std::end(clusters[cid]),
                             std::begin(follower->hits_),
                             std::end(follower->hits_));
        // Add follower to the stack so its followers can also get the correct
        // cluster index
        cluster_stack.push(follower_index);
      }
    }
    // for first clusteringloop, we want to save number of clusters before
    // reclustering
    if (clustering_loops_ == 1 && energy_overload)
      initial_cluster_nbr_ = clusters.size();
  endwhile:;
  } while (energy_overload);
  // if we have more than one layer and we are not currently doing CLUE3D
  if (!connecting_layers && nbr_of_layers_ > 1) {
    // Overwrite seed densities' properties with cluster properties
    // Might be cleaner to just create new densities for cluster seeds
    for (auto& seed : layer_seeds) {
      seed->delta_ = std::numeric_limits<float>::max();
      seed->hits_ = clusters[seed->cluster_id_];
      seed->total_energy_ = cluster_energies[seed->cluster_id_];
      seed->index_ = seed_index_;
      seed_index_++;
    }
    // Sort seeds in layer based on energy
    std::sort(layer_seeds.begin(), layer_seeds.end(),
              [](const std::shared_ptr<Density>& a,
                 const std::shared_ptr<Density>& b) {
                return a->total_energy_ > b->total_energy_;
              });
  }
  return clusters;
}  // end of clustering

// Function to connect seeds between layers to form 3D clusters
// ONLY used in CLUE3D
std::vector<std::shared_ptr<CLUE::Density>> CLUE::setupForClue3D() {
  ldmx_log(trace) << "--- LAYER SETUP ---";
  std::vector<std::shared_ptr<CLUE::Density>> densities;
  layer_rho_c_.clear();
  for (int layer = 0; layer < nbr_of_layers_; layer++) {
    ldmx_log(trace) << "  LAYER " << layer << " with " << seeds_[layer].size()
                    << " seeds";
    auto& seeds_in_current_layer = seeds_[layer];
    double highest_energy = 0.;
    for (const auto& current_seed : seeds_in_current_layer) {
      // for each seed in layer
      current_seed->layer_ = layer;
      if (current_seed->total_energy_ > highest_energy)
        highest_energy = current_seed->total_energy_;
      ldmx_log(trace) << "    Density with index " << current_seed->index_
                      << ", energy: " << current_seed->total_energy_
                      << " position (x,y)= {" << current_seed->x_ << ","
                      << current_seed->y_ << ")";
      int depth = 1;
      // decide delta_ and followerof from seeds in previous and next layer_
      // do {
      // depth++;
      if ((layer - depth >= 0) && (layer - depth < seeds_.size())) {
        ldmx_log(trace) << "    Looking at pre-layer: " << layer - depth;
        // look at previous layer
        ldmx_log(trace) << "      In previous layer... ";
        auto& previous_layer = seeds_[layer - depth];
        ldmx_log(trace) << "      Got " << previous_layer.size() << " seeds";
        for (const auto& prev_seed : previous_layer) {
          // for each seed in previous layer
          auto distance_2d_prev = dist(current_seed->x_, current_seed->y_,
                                       prev_seed->x_, prev_seed->y_);
          auto dz_prev = std::abs(current_seed->z_ - prev_seed->z_);
          ldmx_log(trace) << "      DeltaXY to index " << prev_seed->index_
                          << ": " << std::setprecision(4) << distance_2d_prev;
          ldmx_log(trace) << "      DeltaZ to index " << prev_seed->index_
                          << ": " << std::setprecision(4) << dz_prev;
          if (prev_seed->total_energy_ > current_seed->total_energy_ &&
              distance_2d_prev < current_seed->delta_ &&
              dz_prev < current_seed->z_delta_) {
            ldmx_log(trace) << "      New parent: index " << prev_seed->index_
                            << " on layer " << layer - depth << "; energy "
                            << prev_seed->total_energy_;
            ldmx_log(trace) << "      New 2D distance to prev-layer: "
                            << std::setprecision(4) << distance_2d_prev;
            ldmx_log(trace)
                << "      New delta Z: " << std::setprecision(4) << dz_prev;
            current_seed->delta_ = distance_2d_prev;
            current_seed->z_delta_ = dz_prev;
            current_seed->follower_of_ = prev_seed->index_;
          } else if (prev_seed->total_energy_ < current_seed->total_energy_) {
            ldmx_log(trace) << "      Breaking on index " << prev_seed->index_
                            << " with energy " << prev_seed->total_energy_;
            break;
          }
        }
      }

      if (layer + depth < nbr_of_layers_ && layer + depth < seeds_.size()) {
        ldmx_log(trace) << "    Looking at post-layer: " << layer + depth;
        auto& next_layer = seeds_[layer + depth];
        ldmx_log(trace) << "      Got " << next_layer.size() << " seeds";
        for (const auto& next_seed : next_layer) {
          auto distance_2d_next = dist(current_seed->x_, current_seed->y_,
                                       next_seed->x_, next_seed->y_);
          auto dz_next = std::abs(current_seed->z_ - next_seed->z_);
          ldmx_log(trace) << "      DeltaXY to index " << next_seed->index_
                          << ": " << std::setprecision(4) << distance_2d_next;
          ldmx_log(trace) << "      DeltaZ to index_" << next_seed->index_
                          << ": " << std::setprecision(4) << dz_next;
          if (next_seed->total_energy_ > current_seed->total_energy_ &&
              distance_2d_next < current_seed->delta_ &&
              dz_next < current_seed->z_delta_) {
            ldmx_log(trace) << "      New parent: index_" << next_seed->index_
                            << " on layer " << layer + depth << "; energy "
                            << next_seed->total_energy_;
            ldmx_log(trace) << "      New 2D distance to next-layer: "
                            << std::setprecision(4) << distance_2d_next;
            ldmx_log(trace)
                << "      New delta_Z: " << std::setprecision(4) << dz_next;
            current_seed->delta_ = distance_2d_next;
            current_seed->z_delta_ = dz_next;
            current_seed->follower_of_ = next_seed->index_;
          } else if (next_seed->total_energy_ < current_seed->total_energy_) {
            ldmx_log(trace) << "      Breaking on index_" << next_seed->index_
                            << " with energy " << next_seed->total_energy_;
            break;
          }
        }  // end loop on seeds in next layer
      }  // end of looking at next layer
      // } while (currentLayer[i]->layerFollowerOf == -1 && (layer - depth >=
      // 0 || layer + depth < nbr_of_layers_));
      ldmx_log(trace) << "      Done setting parents.";
      densities.push_back(current_seed);
    }
    // TODO: This 2 needs to be configurable
    layer_rho_c_.push_back(highest_energy / 2);
  }  // end loop over layers
  return densities;
}  // end of setupForClue3D

void CLUE::convertToIntermediateClusters(
    std::vector<std::vector<const ldmx::EcalHit*>>& clusters) {
  // Convert to workingecalclusters to ensure compatibility with
  // EcalClusterProducer
  for (const auto& cluster : clusters) {
    IntermediateCluster intermediate_cluster{},
        intermediate_cluster_first_layer{};

    for (const auto& hit : cluster) {
      intermediate_cluster.add(hit);
      // if hit is in first layer, add to first layer cluster
      ldmx::EcalID ecal_id(hit->getID());
      auto layer = ecal_id.layer();
      intermediate_cluster.setLayer(layer);
      if (layer == 0) {
        intermediate_cluster_first_layer.add(hit);
        intermediate_cluster_first_layer.setLayer(layer);
      }
    }
    final_clusters_.push_back(intermediate_cluster);
    first_layer_centroids_.push_back(intermediate_cluster_first_layer);
    auto cent_x = intermediate_cluster.centroid().x();
    auto cent_y = intermediate_cluster.centroid().y();
    auto event_cent_x = event_centroid_.centroid().x();
    auto event_cent_y = event_centroid_.centroid().y();
    const auto& distance = dist(cent_x, cent_y, event_cent_x, event_cent_y);
    centroid_distances_.push_back(distance);
  }
}

void CLUE::cluster(const std::vector<ldmx::EcalHit>& unsorted_hits, double dc,
                   double rc, double delta_c, double delta_o, int nbr_of_layers,
                   bool reclustering) {
  ldmx_log(info) << "Starting CLUE clustering with parameters:" << "dc " << dc
                 << ", rc " << rc << ", delta_c " << delta_c << ", delta_o "
                 << delta_o << ", nbr_of_layers " << nbr_of_layers
                 << ", reclustering " << reclustering;
  // cutoff distance for local density
  dc_ = dc;
  // min density to promote as seed/max density to demote as outlier
  rhoc_ = rc;
  // min separation distance for seeds
  deltac_ = delta_c;
  // min separation distance for outliers
  deltao_ = delta_o;
  // max distance to parent for both seeds and followers
  dm_ = std::max(delta_c, delta_o);

  // Recluster merged clusters or not
  reclustering_ = reclustering;
  nbr_of_layers_ = nbr_of_layers;

  if (nbr_of_layers_ < 1) {
    // anything below 1 => include all layers
    nbr_of_layers_ = max_layers_;
  } else if (nbr_of_layers_ > max_layers_) {
    ldmx_log(warn) << "nbr_of_layers_ " << nbr_of_layers_
                   << " exceeds max layers " << max_layers_
                   << ", setting to max layers";
    nbr_of_layers_ = max_layers_;
  }

  // first copy *addresses* so we are only ever passing around pointers
  std::vector<const ldmx::EcalHit*> hits;
  hits.reserve(unsorted_hits.size());
  ldmx_log(debug) << "Clustering " << unsorted_hits.size() << " hits";
  for (const auto& unsorted_hit : unsorted_hits) {
    hits.push_back(&unsorted_hit);
  }
  // sort hits by Z position
  ldmx_log(debug) << "Sorting hits by Z position";
  std::sort(hits.begin(), hits.end(),
            [](const ldmx::EcalHit* a, const ldmx::EcalHit* b) {
              return a->getZPos() < b->getZPos();
            });

  seeds_.resize(nbr_of_layers_);

  if (nbr_of_layers_ > 1) {
    ldmx_log(debug) << "Creating layers";
    // returns a vector of layers, with each layer having a vector of hits
    const auto layers = createLayers(hits);
    ldmx_log(debug) << "Doing layer-wise clustering on " << layers.size()
                    << " layers";
    for (int i = 0; i < layers.size(); i++) {
      ldmx_log(trace) << "--- LAYER " << i + 1 << " ---";
      auto densities = setup(layers[i]);
      auto clusters = clustering(densities, false, i);
      convertToIntermediateClusters(clusters);
      // clustering without 3D
    }
    // Below for CLUE3D, comment for just layer clustering
    // This does not work properly yet
    // auto densities = setupForClue3D();
    // auto clusters = clustering(densities, true);
    // convertToIntermediateClusters(clusters);
  } else {
    ldmx_log(debug) << "Only one layer, doing 2D clustering";
    auto densities = setup(hits);
    auto clusters = clustering(densities, false);
    convertToIntermediateClusters(clusters);
  }
}

}  // namespace ecal
