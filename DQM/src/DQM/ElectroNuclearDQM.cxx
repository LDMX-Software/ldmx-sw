
#include "DQM/ElectroNuclearDQM.h"

namespace dqm {

ElectroNuclearDQM::ElectroNuclearDQM(const std::string& name,
                                     framework::Process& process)
    : NuclearDQM(name, process) {}

void ElectroNuclearDQM::configure(framework::config::Parameters& parameters) {
  NuclearDQM::configure(parameters);
}

void ElectroNuclearDQM::findENElectronProperties(
    const ldmx::SimParticle* en_electron,
    const std::vector<const ldmx::SimParticle*>& en_daughters) {
  histograms_.fill("en_electron_energy", en_electron->getEnergy());
  histograms_.fill("en_electron_vertex_x", en_electron->getVertex()[0]);
  histograms_.fill("en_electron_vertex_y", en_electron->getVertex()[1]);
  histograms_.fill("en_electron_vertex_z", en_electron->getVertex()[2]);

  // The EN interaction vertex is where the first EN daughter was created
  if (!en_daughters.empty()) {
    histograms_.fill("en_vertex_x", en_daughters[0]->getVertex()[0]);
    histograms_.fill("en_vertex_y", en_daughters[0]->getVertex()[1]);
    histograms_.fill("en_vertex_z", en_daughters[0]->getVertex()[2]);
  }
}

void ElectroNuclearDQM::findReconstructableKinematics(
    const std::vector<const ldmx::SimParticle*>& daughters) {
  // Apply semi-inclusive acceptance cuts (daughters already sorted by KE desc):
  //   theta < 80 deg from beam axis
  //   |p| > 100 MeV/c  for pi±
  //   |p| > 800 MeV/c  for protons and kaons
  //   KE  > 1000 MeV   for neutrons
  //   KE  > 2000 MeV for pi0
  std::vector<const ldmx::SimParticle*> recon;
  for (const auto* d : daughters) {
    std::vector<double> pvec_raw{d->getMomentum()};
    ROOT::Math::XYZVector pvec(pvec_raw[0], pvec_raw[1], pvec_raw[2]);
    if (pvec.Theta() * (180.0 / 3.14159) >= 80.0) continue;

    auto pdg_id{std::abs(d->getPdgID())};
    double p_mag{pvec.R()};
    double ke{d->getEnergy() - d->getMass()};

    if (pdg_id == 2112) {
      if (ke <= 1000.0) continue;
    } else if (pdg_id == 211) {
      if (p_mag <= 100.0) continue;
    } else if (pdg_id == 2212 || pdg_id == 321 || pdg_id == 130 ||
               pdg_id == 310) {
      if (p_mag <= 800.0) continue;
    } else if (pdg_id == 111) {
      if (ke <= 2000.0) continue;
    } else {
      continue;
    }
    recon.push_back(d);
  }

  int recon_neutron_mult{0}, recon_proton_mult{0};
  int recon_charged_pion_mult{0}, recon_neutral_pion_mult{0};
  double recon_total_ke{0}, recon_total_neutron_ke{0};
  double recon_hardest_ke{-1}, recon_hardest_theta{-1};
  double recon_hardest_n_ke{-1}, recon_hardest_n_theta{-1};
  double recon_hardest_p_ke{-1}, recon_hardest_p_theta{-1};
  double recon_hardest_pi_ke{-1}, recon_hardest_pi_theta{-1};
  double recon_hardest_pi0_ke{-1}, recon_hardest_pi0_theta{-1};

  for (const auto* d : recon) {
    auto pdg_id{std::abs(d->getPdgID())};
    double ke{d->getEnergy() - d->getMass()};
    recon_total_ke += ke;

    std::vector<double> pvec_raw{d->getMomentum()};
    ROOT::Math::XYZVector pvec(pvec_raw[0], pvec_raw[1], pvec_raw[2]);
    auto theta{pvec.Theta() * (180.0 / 3.14159)};

    if (recon_hardest_ke < ke) {
      recon_hardest_ke = ke;
      recon_hardest_theta = theta;
    }

    if (pdg_id == 2112) {
      recon_neutron_mult++;
      recon_total_neutron_ke += ke;
      if (recon_hardest_n_ke < ke) {
        recon_hardest_n_ke = ke;
        recon_hardest_n_theta = theta;
      }
    } else if (pdg_id == 2212) {
      recon_proton_mult++;
      if (recon_hardest_p_ke < ke) {
        recon_hardest_p_ke = ke;
        recon_hardest_p_theta = theta;
      }
    } else if (pdg_id == 211) {
      recon_charged_pion_mult++;
      if (recon_hardest_pi_ke < ke) {
        recon_hardest_pi_ke = ke;
        recon_hardest_pi_theta = theta;
      }
    } else if (pdg_id == 111) {
      recon_neutral_pion_mult++;
      if (recon_hardest_pi0_ke < ke) {
        recon_hardest_pi0_ke = ke;
        recon_hardest_pi0_theta = theta;
      }
    }
  }

  if (!recon.empty()) {
    auto leading_pdg{std::abs(recon[0]->getPdgID())};
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
    histograms_.fill("recon_leading_particle_type", leading_type);
  }

  auto recon_event_type{classifyEvent(recon, 0)};
  histograms_.fill("event_type_recon", static_cast<int>(recon_event_type));

  histograms_.fill("recon_hardest_ke", recon_hardest_ke);
  histograms_.fill("recon_hardest_theta", recon_hardest_theta);
  histograms_.fill("recon_hardest_n_ke", recon_hardest_n_ke);
  histograms_.fill("recon_hardest_n_theta", recon_hardest_n_theta);
  histograms_.fill("recon_hardest_p_ke", recon_hardest_p_ke);
  histograms_.fill("recon_hardest_p_theta", recon_hardest_p_theta);
  histograms_.fill("recon_hardest_pi_ke", recon_hardest_pi_ke);
  histograms_.fill("recon_hardest_pi_theta", recon_hardest_pi_theta);
  histograms_.fill("recon_hardest_pi0_ke", recon_hardest_pi0_ke);
  histograms_.fill("recon_hardest_pi0_theta", recon_hardest_pi0_theta);
  histograms_.fill("en_recon_neutron_mult", recon_neutron_mult);
  histograms_.fill("en_recon_proton_mult", recon_proton_mult);
  histograms_.fill("en_recon_charged_pion_mult", recon_charged_pion_mult);
  histograms_.fill("en_recon_neutral_pion_mult", recon_neutral_pion_mult);
  histograms_.fill("en_recon_total_ke", recon_total_ke);
  histograms_.fill("en_recon_total_neutron_ke", recon_total_neutron_ke);
}

void ElectroNuclearDQM::analyze(const framework::Event& event) {
  auto particle_map{event.getMap<int, ldmx::SimParticle>(
      sim_particles_coll_name_, sim_particles_passname_)};
  if (particle_map.empty()) return;

  // The EN electron is the primary beam electron (recoil)
  auto [trackID, en_electron] = analysis::getRecoil(particle_map);

  // Collect daughters produced by the electronNuclear process, applying the
  // same PDG filtering as PhotoNuclearDQM (exclude photons, heavy nuclei)
  const auto en_daughters{
      findDaughters(particle_map, en_electron,
                    ldmx::SimParticle::ProcessType::electronNuclear)};

  findENElectronProperties(en_electron, en_daughters);

  histograms_.fill("en_particle_mult", en_electron->getDaughters().size());

  if (en_daughters.empty()) {
    ldmx_log(warn) << "No EN daughters found, skipping kinematics";
    return;
  }

  findExtendedKinematics(en_daughters, "en");
  findReconstructableKinematics(en_daughters);
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::ElectroNuclearDQM)
