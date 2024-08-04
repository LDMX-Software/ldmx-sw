#include "Trigger/Event/TrigParticle.h"

ClassImp(trigger::TrigParticle)

    namespace trigger {
  TrigParticle::TrigParticle(LorentzVector p4)
      : p4_(p4),
        vtx_(0, 0, 0),
        end_(0, 0, 0),
        pdgId_(0),
        hwPt_(0),
        hwEta_(0),
        hwPhi_(0),
        hwQual_(0),
        hwIso_(0) {}
  TrigParticle::TrigParticle(LorentzVector p4, Point vtx)
      : p4_(p4),
        vtx_(vtx),
        end_(0, 0, 0),
        pdgId_(0),
        hwPt_(0),
        hwEta_(0),
        hwPhi_(0),
        hwQual_(0),
        hwIso_(0) {}
  TrigParticle::TrigParticle(LorentzVector p4, Point vtx, int pdgId)
      : p4_(p4),
        vtx_(vtx),
        end_(0, 0, 0),
        pdgId_(pdgId),
        hwPt_(0),
        hwEta_(0),
        hwPhi_(0),
        hwQual_(0),
        hwIso_(0) {}

  // TrigParticle::TrigParticle(LorentzVector p4, Point vtx) : p4_(p4),
  // vtx_(vtx), pdgId_(pdgId), hwPt_(hwPt), hwEta_(hwEta), hwPhi_(hwPhi),
  // hwQual_(hwQual), hwIso_(hwIso) {}

}  // namespace trigger
