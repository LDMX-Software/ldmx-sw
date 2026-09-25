#include "Trigger/Event/TrigClusterNNScore.h"

ClassImp(trigger::TrigClusterNNScore);

namespace trigger {
void TrigClusterNNScore::clear() {
  logits_.clear();
  p_bkg_ = 0.0;
}

}  // namespace trigger
