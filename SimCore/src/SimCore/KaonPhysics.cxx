#include "SimCore/KaonPhysics.h"

namespace simcore {
KaonPhysics::KaonPhysics(const G4String& name,
                         const framework::config::Parameters& parameters)
    : G4VPhysicsConstructor(name) {
  kplus_branching_ratios =
      parameters.getParameter<std::vector<double>>("kplus_branching_ratios");
  kminus_branching_ratios =
      parameters.getParameter<std::vector<double>>("kminus_branching_ratios");
  k0l_branching_ratios =
      parameters.getParameter<std::vector<double>>("k0l_branching_ratios");
  k0s_branching_ratios =
      parameters.getParameter<std::vector<double>>("k0s_branching_ratios");
  kplus_lifetime_factor =
      parameters.getParameter<double>("kplus_lifetime_factor");
  kminus_lifetime_factor =
      parameters.getParameter<double>("kminus_lifetime_factor");
  k0l_lifetime_factor = parameters.getParameter<double>("k0l_lifetime_factor");
  k0s_lifetime_factor = parameters.getParameter<double>("k0s_lifetime_factor");
  verbosity = parameters.getParameter<int>("verbosity");
}
void KaonPhysics::setDecayProperties(
    G4ParticleDefinition* kaon, const std::vector<double>& branching_ratios,
    double lifetime_factor) const {
  auto table{kaon->GetDecayTable()};
  if (!table) {
    EXCEPTION_RAISE("KaonPhysics", "Unable to get the decay table from " +
                                       kaon->GetParticleName());
  }
  if (verbosity > 1) {
    ldmx_log(info) << "Decay details (" << kaon->GetParticleName()
                   << ") before setting branching ratios and lifetimes"
                   << std::endl;
    DumpDecayDetails(kaon);
  }
  kaon->SetPDGLifeTime(kaon->GetPDGLifeTime() * lifetime_factor);
  if (kaon == G4KaonZeroLong::Definition()) {
    (*table)[KaonZeroLongDecayChannel::pi0_pi0_pi0]->SetBR(
        branching_ratios[KaonZeroLongDecayChannel::pi0_pi0_pi0]);
    (*table)[KaonZeroLongDecayChannel::pi0_pip_pim]->SetBR(
        branching_ratios[KaonZeroLongDecayChannel::pi0_pip_pim]);
    (*table)[KaonZeroLongDecayChannel::pip_e_nu]->SetBR(
        branching_ratios[KaonZeroLongDecayChannel::pip_e_nu]);
    (*table)[KaonZeroLongDecayChannel::pim_e_nu]->SetBR(
        branching_ratios[KaonZeroLongDecayChannel::pim_e_nu]);
    (*table)[KaonZeroLongDecayChannel::pim_mu_nu]->SetBR(
        branching_ratios[KaonZeroLongDecayChannel::pim_mu_nu]);
    (*table)[KaonZeroLongDecayChannel::pip_mu_nu]->SetBR(
        branching_ratios[KaonZeroLongDecayChannel::pip_mu_nu]);
  } else if (kaon == G4KaonZeroShort::Definition()) {
    (*table)[KaonZeroShortDecayChannel::pip_pim]->SetBR(
        branching_ratios[KaonZeroShortDecayChannel::pip_pim]);
    (*table)[KaonZeroShortDecayChannel::pi0_pi0]->SetBR(
        branching_ratios[KaonZeroShortDecayChannel::pi0_pi0]);
  } else {
    (*table)[ChargedKaonDecayChannel::mu_nu]->SetBR(
        branching_ratios[ChargedKaonDecayChannel::mu_nu]);
    (*table)[ChargedKaonDecayChannel::pi_pi0]->SetBR(
        branching_ratios[ChargedKaonDecayChannel::pi_pi0]);
    (*table)[ChargedKaonDecayChannel::pi_pi_pi]->SetBR(
        branching_ratios[ChargedKaonDecayChannel::pi_pi_pi]);
    (*table)[ChargedKaonDecayChannel::pi_pi0_pi0]->SetBR(
        branching_ratios[ChargedKaonDecayChannel::pi_pi0_pi0]);
    (*table)[ChargedKaonDecayChannel::pi0_e_nu]->SetBR(
        branching_ratios[ChargedKaonDecayChannel::pi0_e_nu]);
    (*table)[ChargedKaonDecayChannel::pi0_mu_nu]->SetBR(
        branching_ratios[ChargedKaonDecayChannel::pi0_mu_nu]);
  }
  if (verbosity > 0) {
    ldmx_log(info) << "Decay details (" << kaon->GetParticleName()
                   << ") after setting branching ratios and lifetimes"
                   << std::endl;
    DumpDecayDetails(kaon);
  }
}
void KaonPhysics::ConstructParticle() {
  auto kaonPlus{G4KaonPlus::Definition()};
  auto kaonMinus{G4KaonMinus::Definition()};
  auto kaonLong{G4KaonZeroLong::Definition()};
  auto kaonShort{G4KaonZeroShort::Definition()};

  if (!kaonPlus || !kaonMinus || !kaonLong || !kaonShort) {
    EXCEPTION_RAISE("KaonPhysics",
                    "Unable to get the charged kaon particle definitions, "
                    "something is very wrong with the configuration.");
  }
  setDecayProperties(kaonPlus, kplus_branching_ratios, kplus_lifetime_factor);
  setDecayProperties(kaonMinus, kminus_branching_ratios,
                     kminus_lifetime_factor);
  setDecayProperties(kaonLong, k0l_branching_ratios, k0l_lifetime_factor);
  setDecayProperties(kaonShort, k0s_branching_ratios, k0s_lifetime_factor);
}

void KaonPhysics::DumpDecayDetails(const G4ParticleDefinition* kaon) const {
  ldmx_log(info) << "Decay table details for " << kaon->GetParticleName()
                 << std::scientific << std::setprecision(15)
                 << " (PDG Lifetime " << kaon->GetPDGLifeTime() << ")"
                 << std::endl;
  const auto table{kaon->GetDecayTable()};
  const int entries{table->entries()};
  for (auto i{0}; i < entries; ++i) {
    const auto channel{(*table)[i]};
    const auto daughters{channel->GetNumberOfDaughters()};
    std::string products{};
    // N-1 to avoid extra " + "
    for (auto j{0}; j < daughters - 1; ++j) {
      products += channel->GetDaughter(j)->GetParticleName();
      products += " + ";
    }
    // Special formatting for last one :)
    products += channel->GetDaughter(daughters - 1)->GetParticleName();
    ldmx_log(info) << "Channel " << i << " (" << kaon->GetParticleName()
                   << " -> " << products << ") Kinematics type "
                   << channel->GetKinematicsName() << " with BR "
                   << channel->GetBR() << std::endl;
  }
}

}  // namespace simcore
