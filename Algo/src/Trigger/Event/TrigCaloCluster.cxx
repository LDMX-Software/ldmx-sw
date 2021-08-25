#include "Trigger/Event/TrigCaloCluster.h"

ClassImp(trigger::TrigCaloCluster)

namespace trigger {
  
  TrigCaloCluster::TrigCaloCluster(float x, float y, float z, float e)
      : x_(x), y_(y), z_(z), e_(e) {}

  void TrigCaloCluster::Clear(){
    x_=0;
    y_=0;
    z_=0;
    e_=0;
    dxdz_=0;
    dydz_=0;
    xe_=0;
    ye_=0;
    ze_=0;
    dxdze_=0;
    dydze_=0;
    is3D_=false;
    layer_=-1;
    firstLayer_=-1;
    lastLayer_=-1;
    depth_=0;
  }

}  // namespace trigger



