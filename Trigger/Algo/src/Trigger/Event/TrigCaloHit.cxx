#include "Trigger/Event/TrigCaloHit.h"

ClassImp(trigger::TrigCaloHit)

    namespace trigger {
  TrigCaloHit::TrigCaloHit(float position_x, float position_y, float position_z,
                           float energy)
      : position_x_(position_x),
        position_y_(position_y),
        position_z_(position_z),
        energy_(energy) {
    layer_ = 0;
    strip_ = 0;
    module_section_ = 0;
  }

}  // namespace trigger
