#include "DQM/DarkBremInteraction.h"

namespace dqm {

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

void DarkBremInteraction::setHistLabels(
    const std::string& name, const std::vector<std::string>& labels) {
  /**
   * We could probably move this into Framework since it is a relatively
   * common task. I could even imagine a way of constructing a StrCategory
   * histogram.
   */
  auto h{histograms_.get(name)};
  for (std::size_t ibin{1}; ibin <= labels.size(); ibin++) {
    h->GetXaxis()->SetBinLabel(ibin, labels[ibin - 1].c_str());
  }
}

void DarkBremInteraction::onProcessStart() {
  setHistLabels("dark_brem_material",
                {"Unknown", "C", "PCB", "Glue", "Si", "Al", "W / LYSO", "PVT"});

  setHistLabels("dark_brem_element",
                {"did not happen", "H 1", "C 6", "O 8", "Na 11", "Si 14",
                 "Ca 20", "Cu 29", "W / LYSO 74", "unlisted"});
}

void DarkBremInteraction::produce(framework::Event& event) {
  histograms_.setWeight(event.getEventHeader().getWeight());
  const auto& particle_map{
      event.getMap<int, ldmx::SimParticle>("SimParticles")};
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
     *
     * This can also happen during development, so I leave a debug
     * printout here to be uncommented when developing the dark
     * brem simulation.
    std::cout << "Event " << e.getEventNumber()
      << " did not have a dark brem occur within it." << std::endl;
     */
    return;
  }

  if (recoil == nullptr or aprime == nullptr or beam == nullptr) {
    // we are going to end processing so let's take our time to
    // construct a nice error message
    std::stringstream err_msg;
    err_msg
        << "Unable to find all necessary particles for DarkBrem interaction."
        << " Missing: [ " << (recoil == nullptr ? "recoil " : "")
        << (aprime == nullptr ? "aprime " : "")
        << (beam == nullptr ? "beam " : "") << "]" << std::endl;
    EXCEPTION_RAISE("BadEvent", err_msg.str());
    return;
  }

  const auto& recoil_p = recoil->getMomentum();
  const auto& aprime_p = aprime->getMomentum();

  std::vector<double> incident_p = recoil_p;
  for (std::size_t i{0}; i < recoil_p.size(); ++i)
    incident_p[i] += aprime_p.at(i);

  double incident_energy = energy(incident_p, recoil->getMass());
  double recoil_energy = energy(recoil_p, recoil->getMass());
  double visible_energy = (beam->getEnergy() - incident_energy) + recoil_energy;

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

  histograms_.fill("dark_brem_z", vtx_z);

  int i_element = 0;
  if (db_material_z > 0) {
    if (known_elements_.find(static_cast<int>(db_material_z)) ==
        known_elements_.end()) {
      i_element = known_elements_.size();
    } else {
      i_element = known_elements_.at(static_cast<int>(db_material_z));
    }
  }

  histograms_.fill("dark_brem_element", i_element);
  histograms_.fill("dark_brem_material", ap_vertex_material);
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, DarkBremInteraction);
