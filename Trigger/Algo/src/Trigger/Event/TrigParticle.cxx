#include "Trigger/Event/TrigParticle.h"

ClassImp(trigger::TrigParticle)

    namespace trigger {
  TrigParticle::TrigParticle(LorentzVector p4)
      : p4_(p4),
        vtx_(0, 0, 0),
        end_(0, 0, 0),
        pdg_id_(0),
        hw_pt_(0),
        hw_eta_(0),
        hw_phi_(0),
        hw_qual_(0),
        hw_iso_(0) {}
  TrigParticle::TrigParticle(LorentzVector p4, Point vtx)
      : p4_(p4),
        vtx_(vtx),
        end_(0, 0, 0),
        pdg_id_(0),
        hw_pt_(0),
        hw_eta_(0),
        hw_phi_(0),
        hw_qual_(0),
        hw_iso_(0) {}
  TrigParticle::TrigParticle(LorentzVector p4, Point vtx, int pdgId)
      : p4_(p4),
        vtx_(vtx),
        end_(0, 0, 0),
        pdg_id_(pdgId),
        hw_pt_(0),
        hw_eta_(0),
        hw_phi_(0),
        hw_qual_(0),
        hw_iso_(0) {}

  // TrigParticle::TrigParticle(LorentzVector p4, Point vtx) : p4_(p4),
  // vtx_(vtx), pdg_id_(pdgId), hw_pt_(hwPt), hw_eta_(hwEta), hw_phi_(hwPhi),
  // hw_qual_(hwQual), hw_iso_(hwIso) {}

}  // namespace trigger
