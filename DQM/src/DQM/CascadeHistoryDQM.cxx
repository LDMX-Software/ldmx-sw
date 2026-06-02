#include "DQM/CascadeHistoryDQM.h"

#include <cmath>

namespace dqm {

CascadeHistoryDQM::CascadeHistoryDQM(const std::string& name,
                                     framework::Process& process)
    : framework::Analyzer(name, process) {}

void CascadeHistoryDQM::configure(framework::config::Parameters& parameters) {
  cascade_coll_name_ = parameters.get<std::string>(
      "cascade_coll_name", "PhotonuclearCascadeHistories");
  cascade_pass_name_ = parameters.get<std::string>("cascade_pass_name", "");
}

void CascadeHistoryDQM::analyze(const framework::Event& event) {
  if (!event.exists(cascade_coll_name_, cascade_pass_name_)) {
    return;
  }

  auto cascade_map = event.getMap<int, ldmx::CascadeHistory>(
      cascade_coll_name_, cascade_pass_name_);
  if (cascade_map.empty()) {
    return;
  }

  histograms_.fill("n_cascades", cascade_map.size());

  for (const auto& [trackId, history] : cascade_map) {
    analyzeCascade(history);
  }
}

void CascadeHistoryDQM::analyzeCascade(const ldmx::CascadeHistory& history) {
  const auto& steps = history.getSteps();
  if (steps.empty()) {
    return;
  }

  // Basic cascade properties
  histograms_.fill("cascade_n_steps", steps.size());
  histograms_.fill("cascade_target_A", history.getTargetA());
  histograms_.fill("cascade_target_Z", history.getTargetZ());

  double incident_energy = history.getIncidentEnergy();
  if (incident_energy > 0) {
    histograms_.fill("incident_photon_energy", incident_energy);
  }

  // Count particles by type and status
  int n_protons = 0, n_neutrons = 0, n_pions = 0, n_kaons = 0, n_other = 0;
  int n_interacted = 0, n_escaped = 0;
  int max_generation = 0;

  // Primary reaction analysis
  int primary_n_protons = 0, primary_n_neutrons = 0;
  int primary_n_piplus = 0, primary_n_piminus = 0, primary_n_pizero = 0;
  int primary_n_kaons = 0, primary_n_other = 0;
  double primary_total_ke = 0;
  double primary_max_ke = 0;

  // De-excitation analysis
  int n_deexcitation = 0;
  int n_deexcitation_neutrons = 0, n_deexcitation_protons = 0;
  int n_deexcitation_gammas = 0, n_deexcitation_alphas = 0;
  double deexcitation_total_energy = 0;

  for (const auto& step : steps) {
    int pdg = step.getPdgId();
    int abs_pdg = std::abs(pdg);
    int category = getParticleCategory(pdg);
    double ke = step.getKineticEnergy();

    switch (category) {
      case 0:
        n_protons++;
        break;
      case 1:
        n_neutrons++;
        break;
      case 2:
      case 3:
      case 4:
        n_pions++;
        break;
      case 5:
        n_kaons++;
        break;
      default:
        n_other++;
        break;
    }

    histograms_.fill("step_pdg_category", category);
    histograms_.fill("step_generation", step.getGeneration());
    histograms_.fill("step_stage", step.getStageInt());
    histograms_.fill("step_ke", ke);
    histograms_.fill("step_zone", step.getZone());

    // Position within nucleus
    double x = step.getX();
    double y = step.getY();
    double z = step.getZ();
    double radius = std::sqrt(x * x + y * y + z * z);
    histograms_.fill("step_radius", radius);
    histograms_.fill("step_x", x);
    histograms_.fill("step_y", y);
    histograms_.fill("step_z", z);

    if (step.getGeneration() > max_generation) {
      max_generation = step.getGeneration();
    }

    if (step.didInteract()) {
      n_interacted++;
    }
    if (step.didEscape()) {
      n_escaped++;
      histograms_.fill("escaped_pdg_category", category);
      histograms_.fill("escaped_ke", ke);
    }

    // Primary reaction products (generation 1, from the initial gamma-nucleon
    // interaction)
    if (step.getStage() == ldmx::CascadeStage::PRIMARY) {
      histograms_.fill("primary_daughter_pdg", category);
      histograms_.fill("primary_daughter_ke", ke);
      primary_total_ke += ke;
      if (ke > primary_max_ke) primary_max_ke = ke;

      if (pdg == 2212)
        primary_n_protons++;
      else if (pdg == 2112)
        primary_n_neutrons++;
      else if (pdg == 211)
        primary_n_piplus++;
      else if (pdg == -211)
        primary_n_piminus++;
      else if (pdg == 111)
        primary_n_pizero++;
      else if (abs_pdg == 321 || abs_pdg == 311 || abs_pdg == 310 ||
               abs_pdg == 130)
        primary_n_kaons++;
      else
        primary_n_other++;
    }

    // De-excitation products
    if (step.getStage() == ldmx::CascadeStage::DEEXCITATION) {
      n_deexcitation++;
      deexcitation_total_energy += ke;
      histograms_.fill("deexcitation_pdg", category);
      histograms_.fill("deexcitation_ke", ke);

      if (pdg == 2112) {
        n_deexcitation_neutrons++;
        histograms_.fill("deexcitation_neutron_ke", ke);
      } else if (pdg == 2212) {
        n_deexcitation_protons++;
        histograms_.fill("deexcitation_proton_ke", ke);
      } else if (pdg == 22) {
        n_deexcitation_gammas++;
        histograms_.fill("deexcitation_gamma_energy", ke);
      } else if (abs_pdg == 1000020040) {  // alpha
        n_deexcitation_alphas++;
        histograms_.fill("deexcitation_alpha_ke", ke);
      }
    }
  }

  // Per-cascade summary
  histograms_.fill("cascade_n_protons", n_protons);
  histograms_.fill("cascade_n_neutrons", n_neutrons);
  histograms_.fill("cascade_n_pions", n_pions);
  histograms_.fill("cascade_n_kaons", n_kaons);
  histograms_.fill("cascade_n_other", n_other);
  histograms_.fill("cascade_n_interacted", n_interacted);
  histograms_.fill("cascade_n_escaped", n_escaped);
  histograms_.fill("cascade_max_generation", max_generation);

  // Primary reaction summary
  int primary_n_daughters =
      primary_n_protons + primary_n_neutrons + primary_n_piplus +
      primary_n_piminus + primary_n_pizero + primary_n_kaons + primary_n_other;
  int primary_n_pions = primary_n_piplus + primary_n_piminus + primary_n_pizero;

  histograms_.fill("primary_n_daughters", primary_n_daughters);
  histograms_.fill("primary_n_protons", primary_n_protons);
  histograms_.fill("primary_n_neutrons", primary_n_neutrons);
  histograms_.fill("primary_n_piplus", primary_n_piplus);
  histograms_.fill("primary_n_piminus", primary_n_piminus);
  histograms_.fill("primary_n_pizero", primary_n_pizero);
  histograms_.fill("primary_n_pions", primary_n_pions);
  histograms_.fill("primary_n_kaons", primary_n_kaons);
  histograms_.fill("primary_n_other", primary_n_other);
  histograms_.fill("primary_total_daughter_ke", primary_total_ke);
  histograms_.fill("primary_max_daughter_ke", primary_max_ke);

  // De-excitation summary
  histograms_.fill("n_deexcitation", n_deexcitation);
  histograms_.fill("n_deexcitation_neutrons", n_deexcitation_neutrons);
  histograms_.fill("n_deexcitation_protons", n_deexcitation_protons);
  histograms_.fill("n_deexcitation_gammas", n_deexcitation_gammas);
  histograms_.fill("n_deexcitation_alphas", n_deexcitation_alphas);
  histograms_.fill("deexcitation_total_energy", deexcitation_total_energy);

  // Excitation and residual nucleus
  histograms_.fill("excitation_energy", history.getExcitationEnergy());
  histograms_.fill("residual_A", history.getResidualA());
  histograms_.fill("residual_Z", history.getResidualZ());
}

int CascadeHistoryDQM::getParticleCategory(int pdgId) const {
  int abs_pdg = std::abs(pdgId);
  if (pdgId == 2212) return 0;  // proton
  if (pdgId == 2112) return 1;  // neutron
  if (pdgId == 211) return 2;   // pi+
  if (pdgId == -211) return 3;  // pi-
  if (pdgId == 111) return 4;   // pi0
  if (abs_pdg == 321 || abs_pdg == 311 || abs_pdg == 310 || abs_pdg == 130)
    return 5;  // kaons
  return 6;    // other
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::CascadeHistoryDQM)
