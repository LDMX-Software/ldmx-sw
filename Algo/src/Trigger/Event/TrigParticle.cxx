#include "Trigger/Event/TrigParticle.h"

ClassImp(trigger::TrigParticle)

namespace trigger {

  TrigParticle::TrigParticle(LorentzVector p4, Point vtx) : p4_(p4), vtx_(vtx) {}
  TrigParticle::TrigParticle(LorentzVector p4, Point vtx, int pdgId) : p4_(p4), vtx_(vtx), pdgId_(pdgId) {}

}  // namespace trigger
