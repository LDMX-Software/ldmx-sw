
#include "DQM/NuclearDQM.h"

namespace dqm {

NuclearDQM::NuclearDQM(const std::string& name, framework::Process& process)
    : framework::Analyzer(name, process) {}

void NuclearDQM::configure(framework::config::Parameters& parameters) {
  count_light_ions_ = parameters.get<bool>("count_light_ions", true);
  sim_particles_coll_name_ =
      parameters.get<std::string>("sim_particles_coll_name", "SimParticles");
  sim_particles_passname_ =
      parameters.get<std::string>("sim_particles_passname", "");
}

std::vector<const ldmx::SimParticle*> NuclearDQM::findDaughters(
    const std::map<int, ldmx::SimParticle>& particleMap,
    const ldmx::SimParticle* parent, int require_process_type) const {
  std::vector<const ldmx::SimParticle*> daughters;

  for (const auto& daughter_track_id : parent->getDaughters()) {
    if (particleMap.count(daughter_track_id) == 0) continue;

    auto daughter{&(particleMap.at(daughter_track_id))};

    if (require_process_type >= 0 &&
        daughter->getProcessType() != require_process_type)
      continue;

    auto pdg_id{daughter->getPdgID()};
    if (pdg_id == 22 ||
        (pdg_id > 10000 && (!count_light_ions_ || !isLightIon(pdg_id))))
      continue;

    daughters.push_back(daughter);
  }

  std::sort(daughters.begin(), daughters.end(),
            [](const auto& lhs, const auto& rhs) {
              return (lhs->getEnergy() - lhs->getMass()) >
                     (rhs->getEnergy() - rhs->getMass());
            });

  return daughters;
}

void NuclearDQM::findParticleKinematics(
    const std::vector<const ldmx::SimParticle*>& daughters,
    const std::string& prefix) {
  double hardest_ke{-1}, hardest_theta{-1};
  double hardest_proton_ke{-1}, hardest_proton_theta{-1};
  double hardest_neutron_ke{-1}, hardest_neutron_theta{-1};
  double hardest_pion_ke{-1}, hardest_pion_theta{-1};
  double total_ke{0};
  double total_neutron_ke{0};
  int neutron_multiplicity{0};

  for (const auto* daughter : daughters) {
    auto pdg_id{daughter->getPdgID()};
    double ke{daughter->getEnergy() - daughter->getMass()};
    total_ke += ke;

    std::vector<double> vec{daughter->getMomentum()};
    ROOT::Math::XYZVector pvec(vec[0], vec[1], vec[2]);
    auto theta{pvec.Theta() * (180 / 3.14159)};

    if (hardest_ke < ke) {
      hardest_ke = ke;
      hardest_theta = theta;
    }

    if (pdg_id == 2112) {
      total_neutron_ke += ke;
      neutron_multiplicity++;
      if (hardest_neutron_ke < ke) {
        hardest_neutron_ke = ke;
        hardest_neutron_theta = theta;
      }
    }

    if (pdg_id == 2212 && hardest_proton_ke < ke) {
      hardest_proton_ke = ke;
      hardest_proton_theta = theta;
    }

    // charged and neutral pions grouped together, matching original PN logic
    if ((std::abs(pdg_id) == 211 || pdg_id == 111) && hardest_pion_ke < ke) {
      hardest_pion_ke = ke;
      hardest_pion_theta = theta;
    }
  }

  histograms_.fill("hardest_ke", hardest_ke);
  histograms_.fill("hardest_theta", hardest_theta);
  histograms_.fill("h_ke_h_theta", hardest_ke, hardest_theta);
  histograms_.fill("hardest_p_ke", hardest_proton_ke);
  histograms_.fill("hardest_p_theta", hardest_proton_theta);
  histograms_.fill("hardest_n_ke", hardest_neutron_ke);
  histograms_.fill("hardest_n_theta", hardest_neutron_theta);
  histograms_.fill("hardest_pi_ke", hardest_pion_ke);
  histograms_.fill("hardest_pi_theta", hardest_pion_theta);

  histograms_.fill(prefix + "_neutron_mult", neutron_multiplicity);
  histograms_.fill(prefix + "_total_ke", total_ke);
  histograms_.fill(prefix + "_total_neutron_ke", total_neutron_ke);
}

void NuclearDQM::findExtendedKinematics(
    const std::vector<const ldmx::SimParticle*>& daughters,
    const std::string& prefix) {
  // EN-specific kinematics: pi± and pi0 tracked separately, plus proton
  // multiplicity, leading-particle-type summary, and all the generic
  // hardest-particle quantities that findParticleKinematics would provide
  // for PN (so EN does not need to call findParticleKinematics at all).
  double hardest_ke{-1}, hardest_theta{-1};
  double hardest_n_ke{-1}, hardest_n_theta{-1};
  double hardest_p_ke{-1}, hardest_p_theta{-1};
  double hardest_pi0_ke{-1}, hardest_pi0_theta{-1};
  double total_ke{0}, total_neutron_ke{0};
  int neutron_multiplicity{0};
  int proton_multiplicity{0};
  int charged_pion_multiplicity{0};
  int neutral_pion_multiplicity{0};

  for (const auto* daughter : daughters) {
    auto pdg_id{daughter->getPdgID()};
    double ke{daughter->getEnergy() - daughter->getMass()};
    total_ke += ke;

    std::vector<double> vec{daughter->getMomentum()};
    ROOT::Math::XYZVector pvec(vec[0], vec[1], vec[2]);
    auto theta{pvec.Theta() * (180 / 3.14159)};

    if (hardest_ke < ke) {
      hardest_ke = ke;
      hardest_theta = theta;
    }

    if (pdg_id == 2112) {
      neutron_multiplicity++;
      total_neutron_ke += ke;
      if (hardest_n_ke < ke) {
        hardest_n_ke = ke;
        hardest_n_theta = theta;
      }
    } else if (pdg_id == 2212) {
      proton_multiplicity++;
      if (hardest_p_ke < ke) {
        hardest_p_ke = ke;
        hardest_p_theta = theta;
      }
    } else if (std::abs(pdg_id) == 211) {
      charged_pion_multiplicity++;
    } else if (pdg_id == 111) {
      neutral_pion_multiplicity++;
      if (hardest_pi0_ke < ke) {
        hardest_pi0_ke = ke;
        hardest_pi0_theta = theta;
      }
    }
  }

  // Leading-particle-type histogram:
  // bins: 0=pi±+X, 1=pi0+X, 2=K±+X, 3=KS/KL+X, 4=p+X, 5=n+X, 6=other+X
  if (!daughters.empty()) {
    auto leading_pdg{std::abs(daughters[0]->getPdgID())};
    int leading_type{6};
    if (leading_pdg == 211)
      leading_type = 0;
    else if (leading_pdg == 111)
      leading_type = 1;
    else if (leading_pdg == 321)
      leading_type = 2;
    else if (leading_pdg == 130 || leading_pdg == 310)
      leading_type = 3;
    else if (leading_pdg == 2212)
      leading_type = 4;
    else if (leading_pdg == 2112)
      leading_type = 5;
    histograms_.fill("leading_particle_type", leading_type);
  }

  histograms_.fill("hardest_ke", hardest_ke);
  histograms_.fill("hardest_theta", hardest_theta);
  histograms_.fill("h_ke_h_theta", hardest_ke, hardest_theta);
  histograms_.fill("hardest_n_ke", hardest_n_ke);
  histograms_.fill("hardest_n_theta", hardest_n_theta);
  histograms_.fill("hardest_p_ke", hardest_p_ke);
  histograms_.fill("hardest_p_theta", hardest_p_theta);
  histograms_.fill("hardest_pi0_ke", hardest_pi0_ke);
  histograms_.fill("hardest_pi0_theta", hardest_pi0_theta);
  histograms_.fill(prefix + "_neutron_mult", neutron_multiplicity);
  histograms_.fill(prefix + "_proton_mult", proton_multiplicity);
  histograms_.fill(prefix + "_charged_pion_mult", charged_pion_multiplicity);
  histograms_.fill(prefix + "_neutral_pion_mult", neutral_pion_multiplicity);
  histograms_.fill(prefix + "_total_ke", total_ke);
  histograms_.fill(prefix + "_total_neutron_ke", total_neutron_ke);
}

void NuclearDQM::findSubleadingKinematics(
    const ldmx::SimParticle* initiator,
    const std::vector<const ldmx::SimParticle*>& daughters,
    EventType eventType) {
  // Note: assumes daughters is sorted by kinetic energy descending

  double subleading_ke{-9999};
  double n_energy{-9999}, energy_diff{-9999}, energy_frac{-9999};

  n_energy = daughters[0]->getEnergy() - daughters[0]->getMass();
  if (daughters.size() > 1) {
    subleading_ke = daughters[1]->getEnergy() - daughters[1]->getMass();
  }
  energy_diff = initiator->getEnergy() - n_energy;
  energy_frac = n_energy / initiator->getEnergy();

  if (eventType == EventType::single_neutron) {
    histograms_.fill("1n_ke:2nd_h_ke", n_energy, subleading_ke);
    histograms_.fill("1n_neutron_energy", n_energy);
    histograms_.fill("1n_energy_diff", energy_diff);
    histograms_.fill("1n_energy_frac", energy_frac);
  } else if (eventType == EventType::two_neutrons) {
    histograms_.fill("2n_n2_energy", subleading_ke);
    auto energy_frac2n = (n_energy + subleading_ke) / initiator->getEnergy();
    histograms_.fill("2n_energy_frac", energy_frac2n);
    histograms_.fill("2n_energy_other", initiator->getEnergy() - energy_frac2n);
  } else if (eventType == EventType::charged_kaon) {
    histograms_.fill("1kp_ke:2nd_h_ke", n_energy, subleading_ke);
    histograms_.fill("1kp_energy", n_energy);
    histograms_.fill("1kp_energy_diff", energy_diff);
    histograms_.fill("1kp_energy_frac", energy_frac);
  } else if (eventType == EventType::klong || eventType == EventType::kshort) {
    histograms_.fill("1k0_ke:2nd_h_ke", n_energy, subleading_ke);
    histograms_.fill("1k0_energy", n_energy);
    histograms_.fill("1k0_energy_diff", energy_diff);
    histograms_.fill("1k0_energy_frac", energy_frac);
  }
}

NuclearDQM::EventType NuclearDQM::classifyEvent(
    const std::vector<const ldmx::SimParticle*>& daughters, double threshold) {
  short n{0}, p{0}, pi{0}, pi0{0}, exotic{0}, k0l{0}, kp{0}, k0s{0};

  for (const auto& daughter : daughters) {
    auto ke{daughter->getEnergy() - daughter->getMass()};
    // daughters are sorted by KE descending; stop when below threshold
    if (ke <= threshold) break;

    auto pdg_id{std::abs(daughter->getPdgID())};
    if (pdg_id == 2112)
      n++;
    else if (pdg_id == 2212)
      p++;
    else if (pdg_id == 211)
      pi++;
    else if (pdg_id == 111)
      pi0++;
    else if (pdg_id == 130)
      k0l++;
    else if (pdg_id == 321)
      kp++;
    else if (pdg_id == 310)
      k0s++;
    else
      exotic++;
  }

  int kaons{k0l + kp + k0s};
  int nucleons{n + p};
  int pions{pi + pi0};
  int count{nucleons + pions + exotic + kaons};

  if (count == 0) return EventType::nothing_hard;

  if (count == 1) {
    if (n == 1) return EventType::single_neutron;
    if (p == 1) return EventType::single_proton;
    if (pi0 == 1) return EventType::single_neutral_pion;
    if (pi == 1) return EventType::single_charged_pion;
  }
  if (count == 2) {
    if (n == 2) return EventType::two_neutrons;
    if (n == 1 && p == 1) return EventType::proton_neutron;
    if (p == 2) return EventType::two_protons;
    if (pi == 2) return EventType::two_charged_pions;
    if (pi == 1 && nucleons == 1)
      return EventType::single_charged_pion_and_nucleon;
    if (pi0 == 1 && nucleons == 1)
      return EventType::single_neutral_pion_and_nucleon;
  }
  if (count == 3) {
    if (pi == 1 && nucleons == 2)
      return EventType::single_charged_pion_and_two_nucleons;
    if (pi == 2 && nucleons == 1)
      return EventType::two_charged_pions_and_nucleon;
    if (pi0 == 1 && nucleons == 2)
      return EventType::single_neutral_pion_and_two_nucleons;
    if (pi0 == 1 && nucleons == 1 && pi == 1)
      return EventType::single_neutral_pion_charged_pion_and_nucleon;
  }
  if (count >= 3 && count == n) return EventType::three_or_more_neutrons;

  if (kaons == 1) {
    if (k0l == 1) return EventType::klong;
    if (kp == 1) return EventType::charged_kaon;
    if (k0s == 1) return EventType::kshort;
  }
  if (exotic == count && count != 0) return EventType::exotics;

  return EventType::multibody;
}

NuclearDQM::CompactEventType NuclearDQM::classifyCompactEvent(
    const ldmx::SimParticle* initiator,
    const std::vector<const ldmx::SimParticle*>& daughters, double threshold) {
  short n{0}, n_t{0}, k0l{0}, kp{0}, k0s{0}, soft{0};

  for (const auto& daughter : daughters) {
    auto ke{daughter->getEnergy() - daughter->getMass()};
    auto pdg_id{std::abs(daughter->getPdgID())};

    if (ke < 500) {
      soft++;
      continue;
    }

    if (ke >= 0.8 * initiator->getEnergy()) {
      if (pdg_id == 2112)
        n++;
      else if (pdg_id == 130)
        k0l++;
      else if (pdg_id == 321)
        kp++;
      else if (pdg_id == 310)
        k0s++;
      continue;
    }

    if (pdg_id == 2112 && ke > threshold) n_t++;
  }

  int neutral_kaons{k0l + k0s};
  if (n != 0) return CompactEventType::single_neutron;
  if (kp != 0) return CompactEventType::single_charged_kaon;
  if (neutral_kaons != 0) return CompactEventType::single_neutral_kaon;
  if (n_t == 2) return CompactEventType::two_neutrons;
  if (soft == static_cast<short>(daughters.size()))
    return CompactEventType::soft;
  return CompactEventType::other;
}

}  // namespace dqm
