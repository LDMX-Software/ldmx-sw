
#include "DQM/PhotoNuclearDQM.h"

namespace dqm {

PhotoNuclearDQM::PhotoNuclearDQM(const std::string &name,
                                 framework::Process &process)
    : framework::Analyzer(name, process) {}

std::vector<const ldmx::SimParticle *> PhotoNuclearDQM::findDaughters(
    const std::map<int, ldmx::SimParticle> &particleMap,
    const ldmx::SimParticle *parent) const {
  std::vector<const ldmx::SimParticle *> pn_daughters;
  for (const auto &daughter_track_id : parent->getDaughters()) {
    // skip daughters that weren't saved
    if (particleMap.count(daughter_track_id) == 0) {
      continue;
    }

    auto daughter{&(particleMap.at(daughter_track_id))};

    // Get the PDG ID
    auto pdg_id{daughter->getPdgID()};

    // Ignore photons and nuclei
    if (pdg_id == 22 ||
        (pdg_id > 10000 && (!count_light_ions_ || !isLightIon(pdg_id)))) {
      continue;
    }
    pn_daughters.push_back(daughter);
  }

  std::sort(pn_daughters.begin(), pn_daughters.end(),
            [](const auto &lhs, const auto &rhs) {
              double lhs_ke = lhs->getEnergy() - lhs->getMass();
              double rhs_ke = rhs->getEnergy() - rhs->getMass();
              return lhs_ke > rhs_ke;
            });

  return pn_daughters;
}
void PhotoNuclearDQM::findRecoilProperties(const ldmx::SimParticle *recoil) {
  histograms_.fill("recoil_vertex_x", recoil->getVertex()[0]);
  histograms_.fill("recoil_vertex_y", recoil->getVertex()[1]);
  histograms_.fill("recoil_vertex_z", recoil->getVertex()[2]);
  histograms_.fill("recoil_vertex_x:recoil_vertex_y", recoil->getVertex()[0],
                   recoil->getVertex()[1]);
}
void PhotoNuclearDQM::findParticleKinematics(
    const std::vector<const ldmx::SimParticle *> &pnDaughters) {
  double hardest_ke{-1}, hardest_theta{-1};
  double hardest_proton_ke{-1}, hardest_proton_theta{-1};
  double hardest_neutron_ke{-1}, hardest_neutron_theta{-1};
  double hardest_pion_ke{-1}, hardest_pion_theta{-1};
  double total_ke{0};
  double total_neutron_ke{0};
  int neutron_multiplicity{0};
  // Loop through all of the PN daughters and extract kinematic
  // information.
  for (const auto *daughter : pnDaughters) {
    // skip daughters that weren't saved

    // Get the PDG ID
    auto pdg_id{daughter->getPdgID()};

    // Calculate the kinetic energy
    double ke{daughter->getEnergy() - daughter->getMass()};
    total_ke += ke;

    std::vector<double> vec{daughter->getMomentum()};
    ROOT::Math::XYZVector pvec(vec[0], vec[1], vec[2]);

    //  Calculate the polar angle
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

    if ((pdg_id == 2212) && (hardest_proton_ke < ke)) {
      hardest_proton_ke = ke;
      hardest_proton_theta = theta;
    }

    if (((std::abs(pdg_id) == 211) || (pdg_id == 111)) &&
        (hardest_pion_ke < ke)) {
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

  histograms_.fill("pn_neutron_mult", neutron_multiplicity);
  histograms_.fill("pn_total_ke", total_ke);
  histograms_.fill("pn_total_neutron_ke", total_neutron_ke);
}

void PhotoNuclearDQM::findSubleadingKinematics(
    const ldmx::SimParticle *pnGamma,
    const std::vector<const ldmx::SimParticle *> &pnDaughters,
    const PhotoNuclearDQM::EventType eventType) {
  // Note: Assumes sorted by energy

  double subleading_ke{-9999};
  double n_energy{-9999}, energy_diff{-9999}, energy_frac{-9999};

  n_energy = pnDaughters[0]->getEnergy() - pnDaughters[0]->getMass();
  subleading_ke = -9999;
  if (pnDaughters.size() > 1) {
    subleading_ke = pnDaughters[1]->getEnergy() - pnDaughters[1]->getMass();
  }
  energy_diff = pnGamma->getEnergy() - n_energy;
  energy_frac = n_energy / pnGamma->getEnergy();

  if (eventType == EventType::single_neutron) {
    histograms_.fill("1n_ke:2nd_h_ke", n_energy, subleading_ke);
    histograms_.fill("1n_neutron_energy", n_energy);
    histograms_.fill("1n_energy_diff", energy_diff);
    histograms_.fill("1n_energy_frac", energy_frac);
  } else if (eventType == EventType::two_neutrons) {
    histograms_.fill("2n_n2_energy", subleading_ke);
    auto energy_frac2n = (n_energy + subleading_ke) / pnGamma->getEnergy();
    histograms_.fill("2n_energy_frac", energy_frac2n);
    histograms_.fill("2n_energy_other", pnGamma->getEnergy() - energy_frac2n);

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

void PhotoNuclearDQM::configure(framework::config::Parameters &parameters) {
  count_light_ions_ = parameters.get<bool>("count_light_ions", true);
  sim_particles_coll_name_ =
      parameters.get<std::string>("sim_particles_coll_name");
  sim_particles_passname_ =
      parameters.get<std::string>("sim_particles_passname");
}

void PhotoNuclearDQM::analyze(const framework::Event &event) {
  // Get the particle map from the event.  If the particle map is empty,
  // don't process the event.
  auto particle_map{event.getMap<int, ldmx::SimParticle>(
      sim_particles_coll_name_, sim_particles_passname_)};
  if (particle_map.size() == 0) {
    return;
  }

  // Get the recoil electron
  auto [trackID, recoil] = analysis::getRecoil(particle_map);
  findRecoilProperties(recoil);

  // Use the recoil electron to retrieve the gamma that underwent a
  // photo-nuclear reaction.
  auto pn_gamma{analysis::getPNGamma(particle_map, recoil, 2500.)};
  if (pn_gamma == nullptr) {
    ldmx_log(warn) << "PN Daughter is lost, skipping";
    return;
  }

  const auto pn_daughters{findDaughters(particle_map, pn_gamma)};

  if (!pn_daughters.empty()) {
    auto pn_vertex_volume{pn_daughters[0]->getVertexVolume()};
    auto pn_interaction_material{pn_daughters[0]->getInteractionMaterial()};

    // Let's start with the PN vertex volume
    if (pn_vertex_volume.find("W_cooling") != std::string::npos) {
      // W_cooling_volume_X
      histograms_.fill("pn_vertex_volume", 2);
    } else if (pn_vertex_volume.find("C_volume") != std::string::npos) {
      // C_volume_X
      histograms_.fill("pn_vertex_volume", 3);
    } else if (pn_vertex_volume.find("PCB_volume") != std::string::npos) {
      histograms_.fill("pn_vertex_volume", 4);
    } else if (pn_vertex_volume.find("CarbonBasePlate") != std::string::npos) {
      // CarbonBasePlate_volume
      histograms_.fill("pn_vertex_volume", 5);
    } else if (pn_vertex_volume.find("W_front") != std::string::npos) {
      // W_front_volume_X
      histograms_.fill("pn_vertex_volume", 6);
    } else if (pn_vertex_volume.find("Si_volume") != std::string::npos) {
      histograms_.fill("pn_vertex_volume", 7);
    } else if (pn_vertex_volume.find("Glue") != std::string::npos) {
      histograms_.fill("pn_vertex_volume", 8);
    } else if (pn_vertex_volume.find("motherboard") != std::string::npos) {
      histograms_.fill("pn_vertex_volume", 9);

    } else {
      ldmx_log(debug) << " Else pn_vertex_volume = " << pn_vertex_volume
                      << " with pn_interaction_material = "
                      << pn_interaction_material;
      histograms_.fill("pn_vertex_volume", 1);
    }

    // Now the interaction material
    if (pn_interaction_material.find("Silicon") != std::string::npos ||
        pn_interaction_material.find("G4_Si") != std::string::npos) {
      histograms_.fill("pn_interaction_material", 2);
    } else if (pn_interaction_material.find("Tungsten") != std::string::npos ||
               pn_interaction_material.find("G4_W") != std::string::npos) {
      histograms_.fill("pn_interaction_material", 3);
    } else if (pn_interaction_material.find("FR4") != std::string::npos) {
      // Glass epoxy
      histograms_.fill("pn_interaction_material", 4);
    } else if (pn_interaction_material.find("Steel") != std::string::npos) {
      histograms_.fill("pn_interaction_material", 5);
    } else if (pn_interaction_material.find("Epoxy") != std::string::npos) {
      // CarbonEpoxyComposite
      histograms_.fill("pn_interaction_material", 6);
    } else if (pn_interaction_material.find("Scintillator") !=
               std::string::npos) {
      // This is the notion for PVT
      histograms_.fill("pn_interaction_material", 7);
    } else if (pn_interaction_material.find("Glue") != std::string::npos) {
      // This is the notion for PVT
      histograms_.fill("pn_interaction_material", 8);
    } else if (pn_interaction_material.find("Air") != std::string::npos ||
               pn_interaction_material.find("G4_AIR") != std::string::npos) {
      // Air
      histograms_.fill("pn_interaction_material", 9);
    } else {
      ldmx_log(debug) << " Else pn_interaction_material = "
                      << pn_interaction_material
                      << " with pn_vertex_volume = " << pn_vertex_volume;
      histograms_.fill("pn_interaction_material", 1);
    }
  } else {
    histograms_.fill("pn_vertex_volume", 0);
    histograms_.fill("pn_interaction_material", 0);
  }

  findParticleKinematics(pn_daughters);

  histograms_.fill("pn_particle_mult", pn_gamma->getDaughters().size());
  histograms_.fill("pn_gamma_energy", pn_gamma->getEnergy());
  histograms_.fill("pn_gamma_int_x", pn_gamma->getEndPoint()[0]);
  histograms_.fill("pn_gamma_int_y", pn_gamma->getEndPoint()[1]);
  histograms_.fill("pn_gamma_int_x:pn_gamma_int_y", pn_gamma->getEndPoint()[0],
                   pn_gamma->getEndPoint()[1]);
  histograms_.fill("pn_gamma_int_z", pn_gamma->getEndPoint()[2]);
  histograms_.fill("pn_gamma_vertex_x", pn_gamma->getVertex()[0]);
  histograms_.fill("pn_gamma_vertex_y", pn_gamma->getVertex()[1]);
  histograms_.fill("pn_gamma_vertex_z", pn_gamma->getVertex()[2]);

  // Classify the event
  auto event_type{classifyEvent(pn_daughters, 200)};
  auto event_type500_me_v{classifyEvent(pn_daughters, 500)};
  auto event_type2000_me_v{classifyEvent(pn_daughters, 2000)};

  auto event_type_comp{classifyCompactEvent(pn_gamma, pn_daughters, 200)};
  auto event_type_comp500_me_v{
      classifyCompactEvent(pn_gamma, pn_daughters, 500)};
  auto event_type_comp2000_me_v{
      classifyCompactEvent(pn_gamma, pn_daughters, 2000)};

  histograms_.fill("event_type", static_cast<int>(event_type));
  histograms_.fill("event_type_500mev", static_cast<int>(event_type500_me_v));
  histograms_.fill("event_type_2000mev", static_cast<int>(event_type2000_me_v));

  histograms_.fill("event_type_compact", static_cast<int>(event_type_comp));
  histograms_.fill("event_type_compact_500mev",
                   static_cast<int>(event_type_comp500_me_v));
  histograms_.fill("event_type_compact_2000mev",
                   static_cast<int>(event_type_comp2000_me_v));

  switch (event_type) {
    case EventType::single_neutron:
      if (pn_daughters.size() > 1) {
        auto second_hardest_pdg_id{abs(pn_daughters[1]->getPdgID())};
        auto n_event_type{-10};
        if (second_hardest_pdg_id == 2112) {
          n_event_type = 0;  // n + n
        } else if (second_hardest_pdg_id == 2212) {
          n_event_type = 1;  // p + n
        } else if (second_hardest_pdg_id == 211) {
          n_event_type = 2;  // Pi+/- + n
        } else if (second_hardest_pdg_id == 111) {
          n_event_type = 3;  // Pi0 + n
        } else {
          n_event_type = 4;  // other
        }
        histograms_.fill("1n_event_type", n_event_type);
      }
      [[fallthrough]];  // Remaining code is important for 1n as well
    case EventType::two_neutrons:
    case EventType::charged_kaon:
    case EventType::klong:
    case EventType::kshort:
      findSubleadingKinematics(pn_gamma, pn_daughters, event_type);
      break;
    default:  // Nothing to do
      break;
  }
}

PhotoNuclearDQM::EventType PhotoNuclearDQM::classifyEvent(
    const std::vector<const ldmx::SimParticle *> daughters, double threshold) {
  short n{0}, p{0}, pi{0}, pi0{0}, exotic{0}, k0l{0}, kp{0}, k0s{0};

  // Loop through all of the PN daughters and extract kinematic
  // information.
  for (const auto &daughter : daughters) {
    // Calculate the kinetic energy
    auto ke{daughter->getEnergy() - daughter->getMass()};

    // Assuming the daughters are sorted by kinetic energy, if the kinetic
    // energy is below threshold, we don't need to look at any further
    // particles.
    if (ke <= threshold) {
      break;
    }

    // Get the PDG ID
    auto pdg_id{abs(daughter->getPdgID())};

    if (pdg_id == 2112) {
      n++;
    } else if (pdg_id == 2212) {
      p++;
    } else if (pdg_id == 211) {
      pi++;
    } else if (pdg_id == 111) {
      pi0++;
    } else if (pdg_id == 130) {
      k0l++;
    } else if (pdg_id == 321) {
      kp++;
    } else if (pdg_id == 310) {
      k0s++;
    } else {
      exotic++;
    }
  }

  int kaons = k0l + kp + k0s;
  int nucleons = n + p;
  int pions = pi + pi0;
  int count = nucleons + pions + exotic + kaons;

  if (count == 0) {
    return EventType::nothing_hard;
  }
  if (count == 1) {
    if (n == 1) {
      return EventType::single_neutron;
    } else if (p == 1) {
      return EventType::single_proton;
    } else if (pi0 == 1) {
      return EventType::single_neutral_pion;
    } else if (pi == 1) {
      return EventType::single_charged_pion;
    }
  }
  if (count == 2) {
    if (n == 2) {
      return EventType::two_neutrons;
    } else if (n == 1 && p == 1) {
      return EventType::proton_neutron;
    } else if (p == 2) {
      return EventType::two_protons;
    } else if (pi == 2) {
      return EventType::two_charged_pions;
    } else if (pi == 1 && nucleons == 1) {
      return EventType::single_charged_pion_and_nucleon;
    } else if (pi0 == 1 && nucleons == 1) {
      return EventType::single_neutral_pion_and_nucleon;
    }
  }

  if (count == 3) {
    if (pi == 1 && nucleons == 2) {
      return EventType::single_charged_pion_and_two_nucleons;
    } else if (pi == 2 && nucleons == 1) {
      return EventType::two_charged_pions_and_nucleon;
    }  // else
    else if (pi0 == 1 && nucleons == 2) {
      return EventType::single_neutral_pion_and_two_nucleons;
    } else if (pi0 == 1 && nucleons == 1 && pi == 1) {
      return EventType::single_neutral_pion_charged_pion_and_nucleon;
    }
  }
  if (count >= 3 && count == n) {
    return EventType::three_or_more_neutrons;
  }

  if (kaons == 1) {
    if (k0l == 1) {
      return EventType::klong;
    } else if (kp == 1) {
      return EventType::charged_kaon;
    } else if (k0s == 1) {
      return EventType::kshort;
    }
  }
  if (exotic == count && count != 0) {
    return EventType::exotics;
  }

  return EventType::multibody;
}

PhotoNuclearDQM::CompactEventType PhotoNuclearDQM::classifyCompactEvent(
    const ldmx::SimParticle *pnGamma,
    const std::vector<const ldmx::SimParticle *> daughters, double threshold) {
  short n{0}, n_t{0}, k0l{0}, kp{0}, k0s{0}, soft{0};

  // Loop through all of the PN daughters and extract kinematic
  // information.
  for (const auto &daughter : daughters) {
    // Calculate the kinetic energy
    auto ke{daughter->getEnergy() - daughter->getMass()};

    // Get the PDG ID
    auto pdg_id{abs(daughter->getPdgID())};

    if (ke < 500) {
      soft++;
      continue;
    }

    if (ke >= 0.8 * pnGamma->getEnergy()) {
      if (pdg_id == 2112) {
        n++;
      } else if (pdg_id == 130) {
        k0l++;
      } else if (pdg_id == 321) {
        kp++;
      } else if (pdg_id == 310) {
        k0s++;
      }
      continue;
    }

    if ((pdg_id == 2112) && ke > threshold) {
      n_t++;
    }
  }

  int neutral_kaons{k0l + k0s};

  if (n != 0) {
    return PhotoNuclearDQM::CompactEventType::single_neutron;
  }
  if (kp != 0) {
    return PhotoNuclearDQM::CompactEventType::single_charged_kaon;
  }
  if (neutral_kaons != 0) {
    return PhotoNuclearDQM::CompactEventType::single_neutral_kaon;
  }
  if (n_t == 2) {
    return PhotoNuclearDQM::CompactEventType::two_neutrons;
  }
  if (soft == daughters.size()) {
    return PhotoNuclearDQM::CompactEventType::soft;
  }

  return PhotoNuclearDQM::CompactEventType::other;
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::PhotoNuclearDQM)
