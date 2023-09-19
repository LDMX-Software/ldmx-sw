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

  TrigCaloCluster(float x, float y, float z, float e=0);

  virtual ~TrigCaloCluster() = default;

  bool operator<(const TrigCaloCluster &h) { return e_ < h.e_; }

  void Clear();
  
  void setEnergy(float e) { e_ = e; }
  void setXYZ(float x, float y, float z) { x_=x;  y_=y;  z_=z; }
  void setXYZerr(float xe, float ye, float ze) { xe_=xe;  ye_=ye;  ze_=ze; }
  void setdxdz(float dxdz){ dxdz_=dxdz; }
  void setdydz(float dydz){ dydz_=dydz; }
  void setdxdze(float dxdze){ dxdze_=dxdze; }
  void setdydze(float dydze){ dydze_=dydze; }
  void set3D(bool x){ is3D_=x; }
  void setLayer(int l){ layer_=l; }
  void setFirstLayer(int l){ firstLayer_=l; }
  void setLastLayer(int l){ lastLayer_=l; }
  void setDepth(int d){ depth_=d; }
  void setNTP(int n){ nTP_=n; }

  float x() const { return x_; }
  float y() const { return y_; }
  float z() const { return z_; }
  float e() const { return e_; }
  float xe() const { return xe_; }
  float ye() const { return ye_; }
  float ze() const { return ze_; }
  float energy() const { return e_; }
  float dxdz() const { return dxdz_; }
  float dydz() const { return dydz_; }
  int nTP() const { return nTP_; }
  int depth() const { return depth_; }
  
 private:
  
  float x_{0};
  float y_{0};
  float z_{0};
  float e_{0};
  float dxdz_{0};
  float dydz_{0};
  // rms
  float xe_{0};
  float ye_{0};
  float ze_{0};
  float dxdze_{0};
  float dydze_{0};
  bool is3D_{true};
  int nTP_{0};
  // 2d attributes
  int layer_{-1};
  // 3d attributes
  int firstLayer_{-1};
  int lastLayer_{-1};
  int depth_{0};

  /// ROOT Dictionary class definition macro
  ClassDef(TrigCaloCluster, 1);
};
}  // namespace trigger

#endif  // TRIGGER_EVENT_TRIGCALOCLUSTER_H
