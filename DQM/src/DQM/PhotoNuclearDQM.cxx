
#include "DQM/PhotoNuclearDQM.h"

namespace dqm {

PhotoNuclearDQM::PhotoNuclearDQM(const std::string& name,
                                 framework::Process& process)
    : NuclearDQM(name, process) {}

void PhotoNuclearDQM::configure(framework::config::Parameters& parameters) {
  NuclearDQM::configure(parameters);
  pn_collection_name_ = parameters.get<std::string>("pn_collection_name",
                                                    "PhotonuclearInteractions");
  pn_pass_name_ = parameters.get<std::string>("pn_pass_name", "");
}

void PhotoNuclearDQM::findRecoilProperties(const ldmx::SimParticle* recoil) {
  histograms_.fill("recoil_vertex_x", recoil->getVertex()[0]);
  histograms_.fill("recoil_vertex_y", recoil->getVertex()[1]);
  histograms_.fill("recoil_vertex_z", recoil->getVertex()[2]);
  histograms_.fill("recoil_vertex_x:recoil_vertex_y", recoil->getVertex()[0],
                   recoil->getVertex()[1]);
}

void PhotoNuclearDQM::analyzeInteractionDetails(const framework::Event& event) {
  // Check if PhotonuclearInteraction collection exists
  if (!event.exists(pn_collection_name_, pn_pass_name_)) {
    // Collection not present - PN tracing was not enabled
    return;
  }

  // Get the PhotonuclearInteraction collection
  const auto& pn_interactions =
      event.getCollection<ldmx::PhotonuclearInteraction>(pn_collection_name_,
                                                         pn_pass_name_);

  // Analyze full cascade genealogy information not available from SimParticles:
  // - Target nucleus Z/A
  // - Immediate cascade multiplicity
  // - Full descendant genealogy tree (ALL particles, not just final state)

  int n_interactions = pn_interactions.size();
  histograms_.fill("pn_interaction_count", n_interactions);

  for (const auto& interaction : pn_interactions) {
    // Target nucleus information - UNIQUE to PhotonuclearInteraction
    histograms_.fill("pn_target_z", interaction.getTargetZ());
    histograms_.fill("pn_target_a", interaction.getTargetA());
    histograms_.fill("pn_target_z:target_a", interaction.getTargetZ(),
                     interaction.getTargetA());

    // Cascade multiplicity: how many particles created in initial cascade
    int n_cascade_secondaries = interaction.getNumImmediateSecondaries();
    histograms_.fill("pn_cascade_multiplicity", n_cascade_secondaries);

    // Full cascade genealogy tree - includes ALL descendants (intermediate +
    // final) This shows the complete cascade evolution, not just final state
    // particles
    auto descendant_map = interaction.getDescendantMap();
    int total_descendants = 0;
    for (const auto& [secondary_id, descendants] : descendant_map) {
      int n_desc = descendants.size();
      total_descendants += n_desc;
      // How many particles (intermediate + final) descended from each cascade
      // particle
      histograms_.fill("pn_descendants_per_cascade_particle", n_desc);
    }
    histograms_.fill("pn_total_final_state_descendants", total_descendants);

    // Cascade compactness: ratio shows what fraction of tree are immediate
    // secondaries ~1.0 → Compact cascade, few branches (most particles are
    // immediate secondaries) ~0.5 → Moderate branching (half the particles are
    // from further generations) ~0.1 → Highly branched cascade with extensive
    // decay/rescatter chains ~0.0 → Extreme branching (the "tungsten bomb"
    // scenarios)
    if (total_descendants > 0) {
      double compactness_ratio =
          static_cast<double>(n_cascade_secondaries) / total_descendants;
      histograms_.fill("pn_cascade_evolution_ratio", compactness_ratio);
    }
  }
}

void PhotoNuclearDQM::analyze(const framework::Event& event) {
  // Get the particle map from the event.  If the particle map is empty,
  // don't process the event.
  auto particle_map{event.getMap<int, ldmx::SimParticle>(
      sim_particles_coll_name_, sim_particles_passname_)};
  if (particle_map.empty()) return;

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

  findParticleKinematics(pn_daughters, "pn");

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
  auto event_type500_mev{classifyEvent(pn_daughters, 500)};
  auto event_type2000_mev{classifyEvent(pn_daughters, 2000)};

  auto event_type_comp{classifyCompactEvent(pn_gamma, pn_daughters, 200)};
  auto event_type_comp500_mev{
      classifyCompactEvent(pn_gamma, pn_daughters, 500)};
  auto event_type_comp2000_mev{
      classifyCompactEvent(pn_gamma, pn_daughters, 2000)};

  histograms_.fill("event_type", static_cast<int>(event_type));
  histograms_.fill("event_type_500mev", static_cast<int>(event_type500_mev));
  histograms_.fill("event_type_2000mev", static_cast<int>(event_type2000_mev));
  histograms_.fill("event_type_compact", static_cast<int>(event_type_comp));
  histograms_.fill("event_type_compact_500mev",
                   static_cast<int>(event_type_comp500_mev));
  histograms_.fill("event_type_compact_2000mev",
                   static_cast<int>(event_type_comp2000_mev));

  switch (event_type) {
    case EventType::single_neutron:
      if (pn_daughters.size() > 1) {
        auto second_hardest_pdg{std::abs(pn_daughters[1]->getPdgID())};
        int n_event_type{-10};
        if (second_hardest_pdg == 2112)
          n_event_type = 0;
        else if (second_hardest_pdg == 2212)
          n_event_type = 1;
        else if (second_hardest_pdg == 211)
          n_event_type = 2;
        else if (second_hardest_pdg == 111)
          n_event_type = 3;
        else
          n_event_type = 4;
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

  // Analyze detailed photonuclear interaction tracking if available
  analyzeInteractionDetails(event);
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::PhotoNuclearDQM)
