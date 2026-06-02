#include "DQM/DarkBremInteraction.h"

namespace dqm {

void DarkBremInteraction::configure(framework::config::Parameters& parameters) {
  particle_coll_name_ = parameters.get<std::string>("particle_coll_name");
  particle_passname_ = parameters.get<std::string>("particle_passname");
}
/**
 * calculate total energy from 3-momentum and mass
 *
 * Since the dark brem model does not include a nucleus, it only is able to
 * conserve momentum, so we need to reconstruct the incident particle's
 * 3-momentum and then use the known particle mass to calculate its total
 * energy.
 *
 * @param[in] p 3-momentum
 * @param[in] m mass
 * @return total energy
 */
static double energy(const std::vector<double>& p, const double& m) {
  return sqrt(p.at(0) * p.at(0) + p.at(1) * p.at(1) + p.at(2) * p.at(2) +
              m * m);
}

/**
 * calculate the sum in quadrature of the passed list of doubles
 *
 * @param[in] list `{`-bracket-enclosed list of doubles to square, sum, and
 * square-root
 */
static double quadsum(const std::initializer_list<double>& list) {
  double sum{0};
  for (const double& elem : list) sum += elem * elem;
  return sqrt(sum);
}

void DarkBremInteraction::produce(framework::Event& event) {
  histograms_.setWeight(event.getEventHeader().getWeight());
  const auto& particle_map{event.getMap<int, ldmx::SimParticle>(
      particle_coll_name_, particle_passname_)};
  const ldmx::SimParticle *recoil{nullptr}, *aprime{nullptr}, *beam{nullptr};
  for (const auto& [track_id, particle] : particle_map) {
    if (track_id == 1) beam = &particle;
    if (particle.getProcessType() ==
        ldmx::SimParticle::ProcessType::eDarkBrem) {
      if (particle.getPdgID() == 622) {
        if (aprime != nullptr) {
          EXCEPTION_RAISE("BadEvent", "Found multiple A' in event.");
        }
        aprime = &particle;
      } else {
        recoil = &particle;
      }
    }
  }

  if (recoil == nullptr and aprime == nullptr) {
    /* dark brem did not occur during the simulation
     *    IF PROPERLY CONFIGURED, this occurs because the simulation
     *    exhausted the maximum number of tries to get a dark brem
     *    to occur. We just leave early so that the entries in the
     *    ntuple are the unphysical numeric minimum.
     */
    ldmx_log(error) << " No dark brem occured in this event";
    return;
  }

  if (recoil == nullptr or aprime == nullptr or beam == nullptr) {
    // we are going to end processing so let's take our time to
    // construct a nice error message
    ldmx_log(fatal)
        << "Unable to find all necessary particles for DarkBrem interaction."
        << " Missing: [ " << (recoil == nullptr ? " recoil " : "")
        << (aprime == nullptr ? " aprime " : "")
        << (beam == nullptr ? " beam " : "") << "]";
    EXCEPTION_RAISE(
        "BadEvent",
        "Unable to find all necessary particles for DarkBrem interaction.");
    return;
  }

  const auto& recoil_p = recoil->getMomentum();
  const auto& aprime_p = aprime->getMomentum();
  ROOT::Math::XYZVector recoil_pvec(recoil_p[0], recoil_p[1], recoil_p[2]);
  ROOT::Math::XYZVector aprime_pvec(aprime_p[0], aprime_p[1], aprime_p[2]);

  std::vector<double> incident_p = recoil_p;
  for (std::size_t i{0}; i < recoil_p.size(); ++i)
    incident_p[i] += aprime_p.at(i);

  double incident_energy = energy(incident_p, recoil->getMass());
  double recoil_energy = energy(recoil_p, recoil->getMass());

  std::vector<double> ap_vertex{aprime->getVertex()};
  std::string ap_vertex_volume{aprime->getVertexVolume()};
  auto ap_vertex_material_it = std::find_if(
      known_materials_.begin(), known_materials_.end(),
      [&](const auto& mat_pair) {
        return ap_vertex_volume.find(mat_pair.first) != std::string::npos;
      });
  int ap_vertex_material = (ap_vertex_material_it != known_materials_.end())
                               ? ap_vertex_material_it->second
                               : 0;

  if (ap_vertex_material == 0) {
    ldmx_log(warn) << "Dark brem interaction occurred in an unknown material: "
                   << ap_vertex_volume;
  }

  int ap_parent_id{-1};
  if (aprime->getParents().size() > 0) {
    ap_parent_id = aprime->getParents().at(0);
  } else {
    ldmx_log(error) << "Found A' without a parent ID!";
  }

  float aprime_energy = energy(aprime_p, aprime->getMass());
  int aprime_genstatus = aprime->getGenStatus();
  double aprime_px{aprime_p.at(0)}, aprime_py{aprime_p.at(1)},
      aprime_pz{aprime_p.at(2)};
  event.add("APrimeEnergy", aprime_energy);
  event.add("APrimePx", aprime_px);
  event.add("APrimePy", aprime_py);
  event.add("APrimePz", aprime_pz);
  event.add("APrimeParentID", ap_parent_id);
  event.add("APrimeGenStatus", aprime_genstatus);

  histograms_.fill("aprime_energy", aprime_energy);
  histograms_.fill("aprime_pt", quadsum({aprime_px, aprime_py}));
  histograms_.fill("aprime_theta", aprime_pvec.Theta() * (180 / 3.14159));

  int recoil_genstatus = recoil->getGenStatus();
  double recoil_px{recoil_p.at(0)}, recoil_py{recoil_p.at(1)},
      recoil_pz{recoil_p.at(2)};
  event.add("RecoilEnergy", recoil_energy);
  event.add("RecoilPx", recoil_px);
  event.add("RecoilPy", recoil_py);
  event.add("RecoilPz", recoil_pz);
  event.add("RecoilGenStatus", recoil_genstatus);

  histograms_.fill("recoil_energy", recoil_energy);
  histograms_.fill("recoil_pt", quadsum({recoil_px, recoil_py}));
  histograms_.fill("recoil_theta", recoil_pvec.Theta() * (180 / 3.14159));

  event.add("IncidentEnergy", incident_energy);
  double incident_px{incident_p.at(0)}, incident_py{incident_p.at(1)},
      incident_pz{incident_p.at(2)};
  event.add("IncidentPx", incident_px);
  event.add("IncidentPy", incident_py);
  event.add("IncidentPz", incident_pz);

  histograms_.fill("incident_energy", incident_energy);
  histograms_.fill("incident_pt", quadsum({incident_px, incident_py}));

  double vtx_x{aprime->getVertex().at(0)}, vtx_y{aprime->getVertex().at(1)},
      vtx_z{aprime->getVertex().at(2)};
  event.add("DarkBremX", vtx_x);
  event.add("DarkBremY", vtx_y);
  event.add("DarkBremZ", vtx_z);
  event.add("DarkBremVertexMaterial", ap_vertex_material);
  float db_material_z =
      event.getEventHeader().getFloatParameter("db_material_z");
  event.add("DarkBremVertexMaterialZ", db_material_z);
  float aprime_conversion_material_z =
      event.getEventHeader().getFloatParameter("aprime_conversion_material_z");

  histograms_.fill("dark_brem_z", vtx_z);

  int i_element = 0;
  if (db_material_z > 0) {
    if (known_elements_.find(static_cast<int>(db_material_z)) ==
        known_elements_.end()) {
      i_element = known_elements_.size();
      ldmx_log(warn)
          << "Dark brem interaction occurred in an unknown element with Z = "
          << db_material_z << ". Using index " << i_element
          << " for this element.";
    } else {
      i_element = known_elements_.at(static_cast<int>(db_material_z));
    }
  }

  histograms_.fill("dark_brem_element", i_element + 0.5);
  histograms_.fill("dark_brem_material", ap_vertex_material + 0.5);

  // Get the daughters of the A' if it decayed within the simulation
  std::vector<int> aprime_daughters = aprime->getDaughters();
  int n_ap_daughters = aprime_daughters.size();
  ldmx_log(debug) << "A' with energy " << aprime->getEnergy() << " and momentum"
                  << " (" << aprime_px << ", " << aprime_py << ", " << aprime_pz
                  << ") GeV " << " has " << n_ap_daughters << " daughters";
  if (n_ap_daughters == 0) {
    histograms_.fill("aprime_daughter_pdgid", 0);
    histograms_.fill("aprime_daughter_material", 0.5);
    histograms_.fill("aprime_daughter_element", 0.5);
  } else {
    // Loop again on the particles to find the daughters of the A'
    for (const auto& [track_id, daughter_particle] : particle_map) {
      for (const auto& primary_daughter : aprime_daughters) {
        if (track_id == primary_daughter) {
          auto const& daughter_p = daughter_particle.getMomentum();
          double daughter_px{daughter_p.at(0)}, daughter_py{daughter_p.at(1)},
              daughter_pz{daughter_p.at(2)};

          ldmx_log(debug) << "  Daughter track ID " << track_id
                          << " with PDG ID " << daughter_particle.getPdgID()
                          << " and energy " << daughter_particle.getEnergy()
                          << " and charge " << daughter_particle.getCharge()
                          << " and mass " << daughter_particle.getMass()
                          << " GeV" << " and momentum (" << daughter_px << ", "
                          << daughter_py << ", " << daughter_pz << ") GeV";
          histograms_.fill("aprime_daughter_energy",
                           daughter_particle.getEnergy());
          histograms_.fill("aprime_daughter_pt",
                           quadsum({daughter_px, daughter_py}));

          // Fill histogram for daughter creation vertex Z position
          double daughter_start_z = daughter_particle.getVertex().at(2);
          histograms_.fill("aprime_daughter_start_z", daughter_start_z);

          // Fill histogram for material where A' daughter was created.
          // Prefer the explicit interaction material; fall back to vertex
          // volume.
          std::string daughter_material_name =
              daughter_particle.getInteractionMaterial();
          std::string daughter_vertex_volume =
              daughter_particle.getVertexVolume();
          int daughter_material = 0;

          if (daughter_material_name.find("Carbon") != std::string::npos) {
            daughter_material = 1;
          } else if (daughter_material_name.find("FR4") != std::string::npos ||
                     daughter_material_name.find("PCB") != std::string::npos ||
                     daughter_vertex_volume.find("motherboard") !=
                         std::string::npos ||
                     daughter_vertex_volume.find("PCB") != std::string::npos) {
            daughter_material = 2;
          } else if (daughter_material_name.find("Glue") != std::string::npos ||
                     daughter_vertex_volume.find("Glue") != std::string::npos ||
                     daughter_vertex_volume.find("CFMix") !=
                         std::string::npos) {
            daughter_material = 3;
          } else if (daughter_material_name.find("Silicon") !=
                         std::string::npos ||
                     daughter_material_name.find("Si") != std::string::npos ||
                     daughter_vertex_volume.find("Si") != std::string::npos ||
                     daughter_vertex_volume.find("Sensor") !=
                         std::string::npos ||
                     daughter_vertex_volume.find("sensor") !=
                         std::string::npos) {
            daughter_material = 4;
          } else if (daughter_material_name.find("Al") != std::string::npos ||
                     daughter_material_name.find("Aluminum") !=
                         std::string::npos ||
                     daughter_vertex_volume.find("strongback") !=
                         std::string::npos ||
                     daughter_vertex_volume.find("support") !=
                         std::string::npos) {
            daughter_material = 5;
          } else if (daughter_material_name.find("W") != std::string::npos ||
                     daughter_material_name.find("Tungsten") !=
                         std::string::npos ||
                     daughter_vertex_volume.find("target") !=
                         std::string::npos ||
                     daughter_vertex_volume.find("W_front_volume") !=
                         std::string::npos ||
                     daughter_vertex_volume.find("W_cooling") !=
                         std::string::npos) {
            daughter_material = 6;
          } else if (daughter_material_name.find("Polyvinyltoluene") !=
                         std::string::npos ||
                     daughter_material_name.find("PVT") != std::string::npos ||
                     daughter_vertex_volume.find("trigger_pad") !=
                         std::string::npos) {
            daughter_material = 7;
          } else if (daughter_material_name.find("Air") != std::string::npos ||
                     daughter_vertex_volume.find("Air") != std::string::npos) {
            daughter_material = 8;
          } else {
            ldmx_log(warn) << "Daughter particle track ID " << track_id
                           << " created in unknown material: "
                           << daughter_material_name
                           << " and vertex volume: " << daughter_vertex_volume;
          }
          histograms_.fill("aprime_daughter_material", daughter_material + 0.5);

          // Fill histogram for element where A' conversion happened.
          // This comes from the conversion process selecting an element in
          // material.
          int daughter_element = 0;
          if (aprime_conversion_material_z > 0) {
            if (known_elements_.find(static_cast<int>(
                    aprime_conversion_material_z)) == known_elements_.end()) {
              daughter_element = known_elements_.size();
            } else {
              daughter_element = known_elements_.at(
                  static_cast<int>(aprime_conversion_material_z));
            }
          }
          histograms_.fill("aprime_daughter_element", daughter_element + 0.5);

          if (daughter_particle.getPdgID() == 11) {
            histograms_.fill("aprime_daughter_pdgid", 1.5);
          } else if (daughter_particle.getPdgID() == -11) {
            histograms_.fill("aprime_daughter_pdgid", 2.5);
          } else if (daughter_particle.getPdgID() == 13) {
            histograms_.fill("aprime_daughter_pdgid", 3.5);
          } else if (daughter_particle.getPdgID() == -13) {
            histograms_.fill("aprime_daughter_pdgid", 4.5);
          } else if (daughter_particle.getPdgID() == 17) {
            histograms_.fill("aprime_daughter_pdgid", 5.5);
          } else if (daughter_particle.getPdgID() == -17) {
            histograms_.fill("aprime_daughter_pdgid", 6.5);
          } else if (daughter_particle.getPdgID() == 211) {
            histograms_.fill("aprime_daughter_pdgid", 7.5);
          } else if (daughter_particle.getPdgID() == -211) {
            histograms_.fill("aprime_daughter_pdgid", 8.5);
          } else {
            histograms_.fill("aprime_daughter_pdgid", 9.5);
          }
        }  // end if track_id matches primary daughter
      }  // end loop over A' daughters
    }  // end loop over particles
  }  // end if n_ap_daughters > 0

  // Get recoil electron daughters if it underwent bremsstrahlung
  std::vector<int> recoil_daughters = recoil->getDaughters();
  int n_recoil_brem_daughters = 0;
  // Loop again on the particles to find the daughters of the recoil electron
  for (const auto& [track_id, daughter_particle] : particle_map) {
    for (const auto& primary_daughter : recoil_daughters) {
      if (track_id == primary_daughter) {
        if (daughter_particle.getEnergy() > (0.2 * recoil->getEnergy()) &&
            (daughter_particle.getPdgID() == 22)) {
          n_recoil_brem_daughters++;
          histograms_.fill("recoil_brem_daughter_energy",
                           daughter_particle.getEnergy());
          histograms_.fill("recoil_brem_daughter_energy_ratio",
                           daughter_particle.getEnergy() / recoil->getEnergy());
          ldmx_log(debug) << "  Recoil electron daughter track ID " << track_id
                          << " with PDG ID " << daughter_particle.getPdgID()
                          << " and energy " << daughter_particle.getEnergy();
        }
      }
    }  // end loop over recoil daughters
  }  // end loop over particles
  histograms_.fill("recoil_brem_daughter_num", n_recoil_brem_daughters);
}  // end of produce

}  // namespace dqm

DECLARE_ANALYZER(dqm::DarkBremInteraction);
