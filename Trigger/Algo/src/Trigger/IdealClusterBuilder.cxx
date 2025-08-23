#include "Trigger/IdealClusterBuilder.h"

namespace trigger {

void ClusterGeometry::addTp(int tid, int cell_id, int module_id, float x,
                            float y) {
  id_map_[tid] = std::make_pair(cell_id, module_id);
  reverse_id_map_[std::make_pair(cell_id, module_id)] = tid;
  positions_[tid] = std::make_pair(x, y);
}
void ClusterGeometry::addNeighbor(int id1, int id2) {
  if (neighbors_.count(id1))
    neighbors_[id1].push_back(id2);
  else
    neighbors_[id1] = {id2};
  if (neighbors_.count(id2))
    neighbors_[id2].push_back(id1);
  else
    neighbors_[id2] = {id1};
  // cout << "Nbs: " << id1 << " " << id2 << endl;
}
bool ClusterGeometry::checkNeighbor(int id1, int id2) {
  // true if neighbors
  auto &ns = neighbors_[id1];
  return std::find(ns.begin(), ns.end(), id2) != ns.end();
}
void ClusterGeometry::initialize() {
  // calculate pairwise distances
  distances_.clear();
  for (auto pair1 = id_map_.begin(); pair1 != id_map_.end(); pair1++) {
    for (auto pair2 = pair1; pair2 != id_map_.end(); pair2++) {
      if (pair1 == pair2) continue;
      auto &id1 = pair1->first;
      auto &id2 = pair2->first;
      auto &xy1 = positions_[id1];
      auto &xy2 = positions_[id2];
      float d =
          sqrt(pow(xy1.first - xy2.first, 2) + pow(xy1.second - xy2.second, 2));
      distances_[std::make_pair(id1, id2)] = d;
      distances_[std::make_pair(id2, id1)] = d;
    }
  }
  // find neighbors
  float n_dist = 1.8 * getDist(getId(0, 0), getId(1, 0));
  for (auto pair1 = id_map_.begin(); pair1 != id_map_.end(); pair1++) {
    for (auto pair2 = pair1; pair2 != id_map_.end(); pair2++) {
      if (pair1 == pair2) continue;
      if (getDist(pair1->first, pair2->first) < n_dist)
        addNeighbor(pair1->first, pair2->first);
    }
  }
  is_initialized_ = true;
}

std::vector<Cluster> IdealClusterBuilder::build2dClustersLayer(
    std::vector<Hit> hits) {
  // Re-index by id
  std::map<int, Hit> hits_by_id;
  for (auto &hit : hits) hits_by_id[hit.id_] = hit;

  if (debug_) {
    cout << "--------\nBuild2dClustersLayer Input Hits" << endl;
    for (auto &hitpair : hits_by_id) hitpair.second.print();
  }

  // Find seeds
  std::vector<Cluster> clusters;
  for (auto &hitpair : hits_by_id) {
    auto &hit = hitpair.second;
    bool is_local_max = true;
    for (auto n : g_->neighbors_[hit.id_]) {
      if (hits_by_id.count(n) && hits_by_id[n].e_ > hit.e_)
        is_local_max = false;
    }
    if (is_local_max && (hit.e_ > seed_thresh_)) {
      hit.used_ = true;
      Cluster c;
      c.hits_.push_back(hit);
      c.e_ = hit.e_;
      c.x_ = hit.x_;
      c.y_ = hit.y_;
      c.z_ = hit.z_;
      c.seed_ = hit.id_;
      c.module_ = g_->id_map_[hit.id_].second;
      c.layer_ = hit.layer_;
      clusters.push_back(c);
    }
  }

  if (debug_) {
    cout << "--------\nAfter seed-finding" << endl;
    for (auto &hitpair : hits_by_id) hitpair.second.print();
    for (auto &c : clusters) c.print();
  }

  // Add neighbors up to the specified limit
  int i_neighbor = 0;
  while (i_neighbor < n_neighbors_) {
    // find (unused) neighbors for all clusters
    std::map<int, std::vector<int> > assoc_clus2hit_i_ds;
    for (int iclus = 0; iclus < clusters.size(); iclus++) {
      auto &clus = clusters[iclus];
      std::vector<int> neighbors;
      for (const auto &hit : clus.hits_) {
        for (auto n : g_->neighbors_[hit.id_]) {
          if (hits_by_id.count(n) && !hits_by_id[n].used_ &&
              hits_by_id[n].e_ > neighb_thresh_) {
            neighbors.push_back(n);
          }
        }
      }
      assoc_clus2hit_i_ds[iclus] = neighbors;
    }

    // check how many clusters to which each hit is assoc
    std::map<int, std::vector<int> > assoc_hit_i_d2clusters;
    for (auto clus2hit_id : assoc_clus2hit_i_ds) {
      auto iclus = clus2hit_id.first;
      auto &hit_i_ds = clus2hit_id.second;
      for (const auto &hit_id : hit_i_ds) {
        if (assoc_hit_i_d2clusters.count(hit_id))
          assoc_hit_i_d2clusters[hit_id].push_back(iclus);
        else
          assoc_hit_i_d2clusters[hit_id] = {iclus};
      }
    }

    // add associated hits to clusters
    //   (w/ optional e-splitting)
    for (auto hit_i_d2clusters : assoc_hit_i_d2clusters) {
      auto hit_id = hit_i_d2clusters.first;
      auto iclusters = hit_i_d2clusters.second;
      if (iclusters.size() == 1) {
        // simply add cell to the cluster
        auto &hit = hits_by_id[hit_id];
        auto iclus = iclusters[0];
        hit.used_ = true;
        clusters[iclus].hits_.push_back(hit);
        clusters[iclus].e_ += hit.e_;
      } else {
        auto &hit = hits_by_id[hit_id];
        hit.used_ = true;
        float esum = 0;
        for (auto iclus : iclusters) {
          esum += clusters[iclus].e_;
        }
        for (auto iclus : iclusters) {
          Hit new_hit = hit;
          if (split_energy_) new_hit.e_ = hit.e_ * clusters[iclus].e_ / esum;
          clusters[iclus].hits_.push_back(new_hit);
          clusters[iclus].e_ += new_hit.e_;
        }
      }
    }

    // rebuild the clusters and return
    for (auto &c : clusters) {
      c.e_ = 0;
      c.x_ = 0;
      c.y_ = 0;
      c.z_ = 0;
      c.xx_ = 0;
      c.yy_ = 0;
      c.zz_ = 0;
      float sumw = 0;
      for (auto hit : c.hits_) {
        // if(debug_) cout << hit.x << " " << hit.y << " " << hit.z << endl;
        c.e_ += hit.e_;
        // cout << "2d: " << h.e << " " << log(h.e/MIN_TP_ENERGY) << endl;
        float w = std::max(0., log(hit.e_ / MIN_TP_ENERGY));  // use log-e wgt
        c.x_ += hit.x_ * w;
        c.y_ += hit.y_ * w;
        c.z_ += hit.z_ * w;
        c.xx_ += hit.x_ * hit.x_ * w;
        c.yy_ += hit.y_ * hit.y_ * w;
        c.zz_ += hit.z_ * hit.z_ * w;
        sumw += w;
      }
      c.x_ /= sumw;
      c.y_ /= sumw;
      c.z_ /= sumw;
      c.xx_ /= sumw;  // now is <x^2>
      c.yy_ /= sumw;
      c.zz_ /= sumw;
      c.xx_ = sqrt(c.xx_ - c.x_ * c.x_);  // now is sqrt(<x^2>-<x>^2)
      c.yy_ = sqrt(c.yy_ - c.y_ * c.y_);
      c.zz_ = sqrt(c.zz_ - c.z_ * c.z_);
    }

    i_neighbor++;

    if (debug_) {
      cout << "--------\nAfter " << i_neighbor << " neighbors" << endl;
      for (auto &hitpair : hits_by_id) hitpair.second.print();
      for (auto &c : clusters) c.print();
    }
  }

  return clusters;
}

void IdealClusterBuilder::build2dClusters() {
  // first partition hits by layer
  std::map<int, std::vector<Hit> > layer_hits;  // id(xy) to Hit
  for (const auto hit : all_hits_) {
    layer_hits[hit.layer_].push_back(hit);
  }

  // run clustering in each layer and add to the list
  for (auto &pair : layer_hits) {
    if (debug_) {
      cout << "Found " << pair.second.size() << " hits in layer " << pair.first
           << endl;
    }
    auto clus = build2dClustersLayer(pair.second);
    all_clusters_.insert(all_clusters_.end(), clus.begin(), clus.end());
  }
}

void IdealClusterBuilder::build3dClusters() {
  if (debug_) {
    cout << "--------\nBuilding 3d clusters" << endl;
  }

  // first partition 2d clusters by layer
  std::vector<std::vector<Cluster> > layer_clusters;
  layer_clusters.resize(LAYER_MAX);  // first 20 layers
  for (auto &clus : all_clusters_) {
    layer_clusters[clus.layer_].push_back(clus);
  }

  // sort by layer
  for (auto &clusters : layer_clusters) eSort(clusters);

  if (debug_) {
    cout << "--------\n3d: sorted 2d inputs" << endl;
    for (auto &clusters : layer_clusters)
      for (auto &c : clusters) c.print(g_);
  }

  // Pass through clusters from layer 0 to last,
  //   starting with highest energy
  bool building = true;
  std::vector<Cluster> clusters3d;
  while (building) {
    // find the seed cluster
    Cluster cluster3d;
    cluster3d.is_2d_ = false;
    cluster3d.first_layer_ = LAYER_SHOWERMAX;
    cluster3d.last_layer_ = LAYER_SHOWERMAX;
    int test_layer;
    for (int ilayer = 0; ilayer < LAYER_MAX; ilayer++) {
      if (LAYER_SHOWERMAX + ilayer < LAYER_MAX) {
        // walk to back of ECal from shower max
        test_layer = LAYER_SHOWERMAX + ilayer;  // 7,8,9,...19
      } else {
        // then to front of ECal
        test_layer = LAYER_MAX - ilayer - 1;  // 20-13-1=6,5,4,...
      }

      auto &clusters2d = layer_clusters[test_layer];

      // still must find the 3d seed
      if (cluster3d.depth_ == 0) {
        // only seed from the middle layers
        if (test_layer > LAYER_SEEDMAX || test_layer < LAYER_SEEDMIN) continue;
        if (clusters2d.size()) {
          if (debug_) {
            cout << " 3d seed: ";
            clusters2d[0].print(g_);
          }
          // cout << "got seed" << endl;
          //  found seed
          cluster3d.clusters2d_ = {clusters2d[0]};
          cluster3d.first_layer_ = test_layer;
          cluster3d.last_layer_ = test_layer;
          cluster3d.depth_ = 1;
          // remove (first) 2d cluster from list
          clusters2d.erase(clusters2d.begin());
        }
      } else {
        // looking to extend the 3d seed
        // grow if 2d seed is a neighbor
        // auto &last_seed2d = clusters3d.back().seed_;
        auto &last_seed2d = cluster3d.clusters2d_.back().seed_;
        // case where we begin extending cluster backward->forward
        if (test_layer == LAYER_SHOWERMAX - 1)
          last_seed2d = cluster3d.clusters2d_.front().seed_;
        if (debug_) {
          // 	cout << cluster3d.clusters2d.size() << endl;
          // 	cluster3d.clusters2d.back().print();
          cout << "  check 3d w/ seed id " << last_seed2d << endl;
          // cout << "   from #cands: " << clusters2d.size() << endl;
        }
        for (int iclus2d = 0; iclus2d < clusters2d.size(); iclus2d++) {
          if (debug_) {
            cout << "  -- " << iclus2d << endl;
            //     clusters2d[iclus2d].print(g_);
            // cout << "  check ext " << seed2d << endl;
          }
          auto &seed2d = clusters2d[iclus2d].seed_;
          if (last_seed2d == seed2d || g_->checkNeighbor(last_seed2d, seed2d)) {
            // if(debug_){
            // 	cout << " extend: ";
            // 	clusters2d[iclus2d].print(g_);
            // }
            // add to 3d cluster
            cluster3d.clusters2d_.push_back(clusters2d[iclus2d]);
            cluster3d.depth_++;
            if (test_layer < cluster3d.first_layer_)
              cluster3d.first_layer_ = test_layer;
            if (test_layer > cluster3d.last_layer_)
              cluster3d.last_layer_ = test_layer;
            // remove from list
            clusters2d.erase(clusters2d.begin() + iclus2d);
            // proceed to next layer
            break;
          }
        }
      }
    }
    // done with all layers. finish or store cluster
    if (cluster3d.depth_ == 0) {
      building = false;
    } else {
      // cout << "storing 3d cluster" << endl;
      if (cluster3d.depth_ >= DEPTH_GOOD) clusters3d.push_back(cluster3d);
    }
  }

  // post-process 3d clusters here
  for (auto &c : clusters3d) {
    c.e_ = 0;
    c.x_ = 0;
    c.y_ = 0;
    c.z_ = 0;
    c.xx_ = 0;
    c.yy_ = 0;
    c.zz_ = 0;
    float sumw = 0;
    for (auto &c2 : c.clusters2d_) {
      c.e_ += c2.e_;
      // cout << "3d: " << c2.e << " " << log(c2.e/MIN_TP_ENERGY) << endl;
      float w = std::max(0., log(c2.e_ / MIN_TP_ENERGY));  // use log-e wgt
      c.x_ += c2.x_ * w;
      c.y_ += c2.y_ * w;
      c.z_ += c2.z_ * w;
      c.xx_ += c2.x_ * c2.x_ * w;
      c.yy_ += c2.y_ * c2.y_ * w;
      c.zz_ += c2.z_ * c2.z_ * w;
      sumw += w;
    }
    // cout << "sum: " << sumw << endl;
    // cout << "x: " << c.x << endl;
    c.x_ /= sumw;
    // cout << "x: " << c.x << endl;
    c.y_ /= sumw;
    c.z_ /= sumw;
    c.xx_ /= sumw;  // now is <x^2>
    c.yy_ /= sumw;
    c.zz_ /= sumw;
    c.xx_ = sqrt(c.xx_ - c.x_ * c.x_);  // now is sqrt(<x^2>-<x>^2)
    c.yy_ = sqrt(c.yy_ - c.y_ * c.y_);
    c.zz_ = sqrt(c.zz_ - c.z_ * c.z_);
    fit(c);  // calc dx/dz, dy/dz
  }

  if (debug_) {
    cout << "--------\nFound 3d clusters" << endl;
    for (auto &c : clusters3d) c.print3d();
  }

  // std::map<int, std::vector<Cluster> > layer_clusters; // id(xy) to Hit
  // for(const auto clus : all_clusters_){
  //     auto l = clus.layer;
  //     if (layer_clusters.count(l)){
  // 	layer_clusters[l].push_back(clus);
  //     } else {
  // 	layer_clusters[l]={clus};
  //     }
  // }
  // // sort by layer
  // for(auto &pair : layer_clusters){
  //     auto &clusters = pair.second;

  //     if(debug_ && clusters.size()>2){
  // 	cout << "--------\nBefore sort " << endl;
  // 	for(auto &c : clusters) c.print();
  //     }
  //     eSort(clusters);
  //     if(debug_ && clusters.size()>2){
  // 	cout << "-- After sort " << endl;
  // 	for(auto &c : clusters) c.print();
  //     }
  // }

  all_clusters_.clear();
  all_clusters_.insert(all_clusters_.begin(), clusters3d.begin(),
                       clusters3d.end());
}

void IdealClusterBuilder::buildClusters() {
  if (debug_) {
    cout << "--------\nAll hits" << endl;
    for (auto &hit : all_hits_) hit.print();
  }

  if (use_towers_) {
    // project hits in z to form towers
    std::map<int, Hit> towers;  // id(xy) to Hit
    for (const auto hit : all_hits_) {
      if (towers.count(hit.id_)) {
        towers[hit.id_].e_ += hit.e_;
        towers[hit.id_].n_sub_hit_++;
      } else {
        towers[hit.id_] = hit;
        towers[hit.id_].layer_ = 0;
        towers[hit.id_].z_ = 0;
        towers[hit.id_].n_sub_hit_ = 1;
      }
    }
    all_hits_.clear();
    for (const auto t : towers) all_hits_.push_back(t.second);

    if (debug_) {
      cout << "--------\nHits after towers" << endl;
      for (auto &hit : all_hits_) hit.print();
    }
  }

  // Cluster the hits in each plane
  build2dClusters();

  if (!use_towers_) {
    build3dClusters();
  }
  eSort(all_clusters_);
};

void IdealClusterBuilder::fit(Cluster &c3) {
  // TODO: think about whether to incorporate uncertainties
  //   into the fit (RMSs), or weight each layer in the fit.

  // skip short clusters
  if (c3.clusters2d_.size() < 4) return;

  // std::vector logE;
  std::vector<float> x;
  std::vector<float> y;
  std::vector<float> z;
  for (const auto &c2 : c3.clusters2d_) {
    // logE.push_back( log(c2.e) );
    x.push_back(c2.x_);
    y.push_back(c2.y_);
    z.push_back(c2.z_);
  }
  TGraph gxz(z.size(), z.data(), x.data());
  auto r_xz = gxz.Fit("pol1", "SQ");  // p0 + x*p1
  c3.dxdz_ = r_xz->Value(1);
  c3.dxdze_ = r_xz->ParError(1);

  TGraph gyz(z.size(), z.data(), y.data());
  auto r_yz = gyz.Fit("pol1", "SQ");  // p0 + x*p1
  c3.dydz_ = r_yz->Value(1);
  c3.dydze_ = r_yz->ParError(1);
}

}  // namespace trigger
