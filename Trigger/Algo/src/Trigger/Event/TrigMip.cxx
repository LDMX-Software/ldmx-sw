#include "Trigger/Event/TrigMip.h"

ClassImp(trigger::TrigMip);

namespace trigger {
void TrigMip::Clear() {
  start_layer_ = 0;
  end_layer_ = 0;
  n_hits_ = 0;
  n_holes_ = 0;
  length_ = 0;
}

}  // namespace trigger
