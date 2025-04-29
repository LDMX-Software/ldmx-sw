#include "Trigger/Event/TrigCaloHit.h"

ClassImp(trigger::TrigCaloHit)

    namespace trigger {
  TrigCaloHit::TrigCaloHit(float x, float y, float z, float e)
      : x_(x), y_(y), z_(z), e_(e) {
    layer_ = 0;
    strip_ = 0;
    modulesection_ = 0;
  }

}  // namespace trigger
