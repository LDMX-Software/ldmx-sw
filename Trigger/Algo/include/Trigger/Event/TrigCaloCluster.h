#ifndef TRIGGER_EVENT_TRIGCALOCLUSTER_H
#define TRIGGER_EVENT_TRIGCALOCLUSTER_H

// ROOT
#include "TObject.h"  //For ClassDef

namespace trigger {

// Forward declaration needed by typedef
class TrigCaloCluster;
typedef std::vector<TrigCaloCluster> TrigCaloClusterCollection;

/**
 * @class TrigCaloCluster
 * @brief Class for clusters built from trigger calo hits
 */
class TrigCaloCluster {
 public:
  TrigCaloCluster() = default;

  TrigCaloCluster(float clus_x, float lus_y, float lus_z, float energy = 0);

  virtual ~TrigCaloCluster() = default;

  bool operator<(const TrigCaloCluster &h) {
    return clus_energy_ < h.clus_energy_;
  }

  void clear();

  void setEnergy(float energy) { clus_energy_ = energy; }
  void setXYZ(float clus_x, float clus_y, float clus_z) {
    clus_x_ = clus_x;
    clus_y_ = clus_y;
    clus_z_ = clus_z;
  }
  void setXYZerr(float x_error, float y_error, float z_error) {
    x_error_ = x_error;
    y_error_ = y_error;
    z_error_ = z_error;
  }
  void setdxdz(float dxdz) { dx_dz_ = dxdz; }
  void setdydz(float dydz) { dy_dz_ = dydz; }
  void setdxdze(float dxdze) { dx_dz_error_ = dxdze; }
  void setdydze(float dydze) { dy_dz_error_ = dydze; }
  void set3D(bool is_3d) { is_3d_ = is_3d; }
  void setLayer(int layer) { layer_ = layer; }
  void setFirstLayer(int layer) { first_layer_ = layer; }
  void setLastLayer(int layer) { last_layer_ = layer; }
  void setDepth(int depth) { depth_ = depth; }
  void setNTP(int num_trigger_primitives) {
    num_trigger_primitives_ = num_trigger_primitives;
  }

  float x() const { return clus_x_; }
  float y() const { return clus_y_; }
  float z() const { return clus_z_; }
  float e() const { return clus_energy_; }
  float xe() const { return x_error_; }
  float ye() const { return y_error_; }
  float ze() const { return z_error_; }
  float energy() const { return clus_energy_; }
  float dxdz() const { return dx_dz_; }
  float dydz() const { return dy_dz_; }
  int nTP() const { return num_trigger_primitives_; }
  int depth() const { return depth_; }

 private:
  float clus_x_{0};
  float clus_y_{0};
  float clus_z_{0};
  float clus_energy_{0};
  float dx_dz_{0};
  float dy_dz_{0};
  // rms
  float x_error_{0};
  float y_error_{0};
  float z_error_{0};
  float dx_dz_error_{0};
  float dy_dz_error_{0};
  bool is_3d_{true};
  int num_trigger_primitives_{0};
  int layer_{-1};
  int first_layer_{-1};
  int last_layer_{-1};
  int depth_{0};

  /// ROOT Dictionary class definition macro
  ClassDef(TrigCaloCluster, 2);
};
}  // namespace trigger

#endif  // TRIGGER_EVENT_TRIGCALOCLUSTER_H
