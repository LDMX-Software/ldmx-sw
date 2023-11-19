#include "SimCore/KaonPhysics.h"

namespace simcore {
KaonPhysics::KaonPhysics(const G4String& name,
                         const framework::config::Parameters& parameters)
    : G4VPhysicsConstructor(name) {
  kplus_branching_ratios =
      parameters.getParameter<std::vector<double>>("kplus_branching_ratios");
  kminus_branching_ratios =
      parameters.getParameter<std::vector<double>>("kminus_branching_ratios");
  kplus_lifetime_factor =
      parameters.getParameter<double>("kplus_lifetime_factor");
  kminus_lifetime_factor =
      parameters.getParameter<double>("kminus_lifetime_factor");
}
void KaonPhysics::setDecayProperties(
    G4ParticleDefinition* kaon, const std::vector<double>& branching_ratios,
    double lifetime_factor) const {
  kaon->SetPDGLifeTime(kaon->GetPDGLifeTime() * lifetime_factor);
  auto table{kaon->GetDecayTable()};
  if (!table) {
    EXCEPTION_RAISE("KaonPhysics", "Unable to get the decay table from " +
                                       kaon->GetParticleName());
  }
  (*table)[KaonDecayChannel::mu_nu]->SetBR(
      branching_ratios[KaonDecayChannel::mu_nu]);
  (*table)[KaonDecayChannel::pi_pi0]->SetBR(
      branching_ratios[KaonDecayChannel::pi_pi0]);
  (*table)[KaonDecayChannel::pi_pi_pi]->SetBR(
      branching_ratios[KaonDecayChannel::pi_pi_pi]);
  (*table)[KaonDecayChannel::pi_pi0_pi0]->SetBR(
      branching_ratios[KaonDecayChannel::pi_pi0_pi0]);
  (*table)[KaonDecayChannel::pi0_e_nu]->SetBR(
      branching_ratios[KaonDecayChannel::pi0_e_nu]);
  (*table)[KaonDecayChannel::pi0_mu_nu]->SetBR(
      branching_ratios[KaonDecayChannel::pi0_mu_nu]);
}

}  // namespace simcore
