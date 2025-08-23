#ifndef IDEALCLUSTERBUILDER_H
#define IDEALCLUSTERBUILDER_H

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <vector>

#include "TFitResult.h"
#include "TGraph.h"

using std::cout;
using std::endl;

namespace trigger {

class ClusterGeometry {
 public:
  bool is_initialized_ = false;

  // cell + module -> TP ID (get from geo service)
  std::map<std::pair<int, int>, int> reverse_id_map_;
  std::map<int, std::pair<int, int> > id_map_;

  // TP ID to X,Y positions in mm
  std::map<int, std::pair<float, float> > positions_;

  // pairwise X,Y distances of all TPs
  std::map<std::pair<int, int>, float> distances_;

  // list of neighbors associated to each TP
  std::map<int, std::vector<int> > neighbors_;

  int getId(int cell_id, int module_id) {
    return reverse_id_map_[std::make_pair(cell_id, module_id)];
  }
  float getDist(int id1, int id2) {
    return distances_[std::make_pair(id1, id2)];
  }

  void addTp(int tid, int cell_id, int module_id, float x, float y);
  void addNeighbor(int id1, int id2);
  bool checkNeighbor(int id1, int id2);

  void initialize();
};

/*
  FWIW, Pictorally, the numbering for the center module is:
  28 29 30 31 | 47 46 44 41
  24 25 26 27 | 45 43 40 37
  20 21 22 23 | 42 39 36 34
  16 17 18 19 | 38 35 33 32
  -----------
  12 13 14 15
  08 09 10 11
  04 05 06 07
  00 01 02 03
*/

class Hit {
 public:
  float x_ = 0;
  float y_ = 0;
  float z_ = 0;
  float e_ = 0;
  int idx_ = -1;
  int cell_id_ = -1;
  int module_id_ = -1;
  int id_ = -1;        // encodes x,y
  int layer_ = 0;      // z
  int n_sub_hit_ = 0;  // for towers
  bool used_ = false;
  void print() {
    cout << "Hit (" << "e= " << e_ << ", id=" << id_ << ", layer= " << layer_
         << ", x= " << x_ << ", y= " << y_ << ", z= " << z_
         << ", nSub= " << n_sub_hit_ << ", used= " << used_ << ")" << endl;
  }
};

class Cluster {
 public:
  std::vector<Hit> hits_;
  std::vector<Cluster> clusters2d_;  // for 3d
  // calculate and store properties...
  float x_ = 0;
  float y_ = 0;
  float z_ = 0;
  // xyz RMS
  float xx_ = 0;
  float yy_ = 0;
  float zz_ = 0;
  float e_ = 0;
  int seed_ = -1;    // id(xy)
  int module_ = -1;  // uses seed
  int layer_ = -1;

  // 3d specific
  bool is_2d_ = true;
  bool used_ = false;
  int first_layer_ = -1;
  int last_layer_ = -1;
  int depth_ = 0;
  float dxdz_ = 0;
  float dxdze_ = 0;
  float dydz_ = 0;
  float dydze_ = 0;

  void print(ClusterGeometry* g = 0) {
    // ClusterGeometry* g;
    if (g == 0) {
      cout << "Cluster (" << "e= " << e_ << ", seed id=" << seed_
           << ", x= " << x_ << ", y= " << y_ << ", z= " << z_
           << ", nHit= " << hits_.size() << ")" << endl;
    } else {
      auto idpair = g->id_map_[seed_];
      cout << "Cluster (" << "e= " << e_ << ", seed id=" << seed_
           << ", cell id=" << idpair.first << ", module id=" << idpair.second
           << ", layer=" << layer_ << ", x= " << x_ << ", y= " << y_
           << ", z= " << z_ << ", nHit= " << hits_.size() << ")" << endl;
    }
  }
  void print3d() {
    cout << "Cluster (" << "e= " << e_ << ", seed id=" << seed_ << ", x= " << x_
         << ", y= " << y_ << ", z= " << z_
         << ", n2dClus= " << clusters2d_.size()
         << ", first_layer=" << first_layer_ << ", depth=" << depth_ << ")"
         << endl;
  }
  void printHits() {
    print();
    for (auto& h : hits_) {
      cout << "  ";
      h.print();
    }
  }
};

class IdealClusterBuilder {
 public:
  virtual ~IdealClusterBuilder() = default;
  std::vector<Hit> all_hits_{};
  std::vector<Cluster> all_clusters_{};
  ClusterGeometry* g_;

  float seed_thresh_ = 0;    // e.g. 100
  float neighb_thresh_ = 0;  // e.g. 100
  int n_neighbors_ = 1;
  bool split_energy_ = true;
  // bool use_towers = true;
  bool use_towers_ = false;
  const int LAYER_MAX = 35;
  const int LAYER_SHOWERMAX = 7;
  const int LAYER_SEEDMIN = 3;
  const int LAYER_SEEDMAX = 15;
  const float MIN_TP_ENERGY = 0.5;  // in MeV
  const int DEPTH_GOOD = 5;
  /* int order3d[LAYER_MAX]={ */
  /*     7,8,6,9,5,10,4,11,3,12,2,13,1,14,0,15,16,17,18,19 */
  /* }; */
  bool debug_ = false;

  void addHit(Hit h) {
    if (h.layer_ >= LAYER_MAX) return;
    auto p = std::make_pair(h.cell_id_, h.module_id_);
    h.id_ = g_->reverse_id_map_[p];
    all_hits_.push_back(h);
  }
  std::vector<Cluster> getClusters() { return all_clusters_; }
  void setClusterGeo(ClusterGeometry* g) { g_ = g; }

  virtual void buildClusters();
  std::vector<Cluster> build2dClustersLayer(std::vector<Hit> hits);
  void build2dClusters();
  void build3dClusters();
  void fit(Cluster& c3);

  /* void BuildClusters(); */
  /* void Cluster2dHits(); */
};

template <class T>
void eSort(std::vector<T>& v) {
  std::sort(v.begin(), v.end(),
            [](const auto& lhs, const auto& rhs) { return lhs.e_ > rhs.e_; });
}

}  // namespace trigger

#endif  // IDEALCLUSTERBUILDER_H
