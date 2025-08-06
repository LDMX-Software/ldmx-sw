#include "Trigger/Event/TrigCaloCluster.h"

ClassImp(trigger::TrigCaloCluster);

namespace trigger {
TrigCaloCluster::TrigCaloCluster(float clus_x, float clus_y, float clus_z,
                                 float clus_energy)
    : clus_x_(clus_x),
      clus_y_(clus_y),
      clus_z_(clus_z),
      clus_energy_(clus_energy) {}

void TrigCaloCluster::clear() {
  clus_x_ = 0;
  clus_y_ = 0;
  clus_z_ = 0;
  clus_energy_ = 0;
  dx_dz_ = 0;
  dy_dz_ = 0;
  x_error_ = 0;
  y_error_ = 0;
  z_error_ = 0;
  dx_dz_error_ = 0;
  dy_dz_error_ = 0;
  is_3d_ = false;
  num_trigger_primitives_ = 0;
  layer_ = -1;
  first_layer_ = -1;
  last_layer_ = -1;
  depth_ = 0;
}

}  // namespace trigger
