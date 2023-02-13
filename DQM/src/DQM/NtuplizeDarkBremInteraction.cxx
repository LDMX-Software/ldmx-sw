#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimParticle.h"

namespace dqm {
class NtuplizeDarkBremInteraction : public framework::Analyzer {
 public:
  NtuplizeDarkBremInteraction(const std::string& n, framework::Process& p)
    : framework::Analyzer(n,p) {}
  virtual void onProcessStart() final override;
  virtual void analyze(const framework::Event& e) final override;
};

void NtuplizeDarkBremInteraction::onProcessStart() {
  getHistoDirectory();
  ntuple_.create("dbint");
  ntuple_.addVar<double>("dbint","x");
  ntuple_.addVar<double>("dbint","y");
  ntuple_.addVar<double>("dbint","z");
  ntuple_.addVar<double>("dbint","weight");
  ntuple_.addVar<int>("dbint","incident_pdg");
  ntuple_.addVar<int>("dbint","incident_genstatus");
  ntuple_.addVar<double>("dbint","incident_mass");
  ntuple_.addVar<double>("dbint","incident_energy");
  ntuple_.addVar<double>("dbint","incident_px");
  ntuple_.addVar<double>("dbint","incident_py");
  ntuple_.addVar<double>("dbint","incident_pz");
  ntuple_.addVar<int>("dbint","recoil_pdg");
  ntuple_.addVar<int>("dbint","recoil_genstatus");
  ntuple_.addVar<double>("dbint","recoil_mass");
  ntuple_.addVar<double>("dbint","recoil_energy");
  ntuple_.addVar<double>("dbint","recoil_px");
  ntuple_.addVar<double>("dbint","recoil_py");
  ntuple_.addVar<double>("dbint","recoil_pz");
  ntuple_.addVar<int>("dbint","aprime_pdg");
  ntuple_.addVar<int>("dbint","aprime_genstatus");
  ntuple_.addVar<double>("dbint","aprime_mass");
  ntuple_.addVar<double>("dbint","aprime_energy");
  ntuple_.addVar<double>("dbint","aprime_px");
  ntuple_.addVar<double>("dbint","aprime_py");
  ntuple_.addVar<double>("dbint","aprime_pz");
  ntuple_.addVar<double>("dbint","visible_energy");
  ntuple_.addVar<double>("dbint","beam_energy");
}

static double energy(const std::vector<double>& p, const double& m) {
  return sqrt(p.at(0)*p.at(0)+ p.at(1)*p.at(1)+ p.at(2)*p.at(2)+ m*m);
}

/**
 * extract the kinematics of the dark brem interaction from the SimParticles
 *
 * Sometimes the electron that undergoes the dark brem is not in a region 
 * where it should be saved (i.e. it is a shower electron inside of the ECal).
 * In this case, we need to reconstruct the incident momentum from the outgoing
 * products (the recoil electron and the dark photon) which should be saved by
 * the biasing filter used during the simulation.
 *
 * Since the dark brem model does not include a nucleus, it only is able to 
 * conserve momentum, so we need to reconstruct the incident particle's 3-momentum
 * and then use the electron mass to calculate its total energy.
 */
void NtuplizeDarkBremInteraction::analyze(const framework::Event& e) {
  const auto& particle_map{e.getMap<int,ldmx::SimParticle>("SimParticles")};
  const ldmx::SimParticle *recoil{nullptr}, *aprime{nullptr}, *beam{nullptr};
  for (const auto& [track_id, particle] : particle_map) {
    if (track_id == 1) beam = &particle;
    if (particle.getProcessType() == ldmx::SimParticle::ProcessType::eDarkBrem) {
      if (particle.getPdgID() == 62) {
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
      << "Unable to final all necessary particles for DarkBrem interaction."
      << " Missing: [ "
      << (recoil == nullptr ? "recoil " : "")
      << (aprime == nullptr ? "aprime " : "")
      << (beam == nullptr ? "beam " : "")
      << "]" << std::endl;
    EXCEPTION_RAISE("BadEvent", err_msg.str());
    return;
  }

  const auto& recoil_p = recoil->getMomentum();
  const auto& aprime_p = aprime->getMomentum();

  std::vector<double> incident_p = recoil_p;
  for (std::size_t i{0}; i < recoil_p.size(); ++i) incident_p[i] += aprime_p.at(i);

  double incident_energy = energy(incident_p, recoil->getMass());
  double recoil_energy = energy(recoil_p, recoil->getMass());
  double visible_energy = (beam->getEnergy() - incident_energy) + recoil_energy;

  ntuple_.setVar<double>("x", aprime->getVertex().at(0));
  ntuple_.setVar<double>("y", aprime->getVertex().at(1));
  ntuple_.setVar<double>("z", aprime->getVertex().at(2));
  ntuple_.setVar<double>("weight", e.getEventWeight());
  ntuple_.setVar<int>("incident_pdg", recoil->getPdgID());
  ntuple_.setVar<int>("incident_genstatus", -1);
  ntuple_.setVar<double>("incident_mass", recoil->getMass());
  ntuple_.setVar<double>("incident_energy", incident_energy);
  ntuple_.setVar<double>("incident_px", incident_p.at(0));
  ntuple_.setVar<double>("incident_py", incident_p.at(1));
  ntuple_.setVar<double>("incident_pz", incident_p.at(2));
  ntuple_.setVar<int>("recoil_pdg", recoil->getPdgID());
  ntuple_.setVar<int>("recoil_genstatus", recoil->getGenStatus());
  ntuple_.setVar<double>("recoil_mass", recoil->getMass());
  ntuple_.setVar<double>("recoil_energy", recoil_energy);
  ntuple_.setVar<double>("recoil_px", recoil_p.at(0));
  ntuple_.setVar<double>("recoil_py", recoil_p.at(1));
  ntuple_.setVar<double>("recoil_pz", recoil_p.at(2));
  ntuple_.setVar<int>("aprime_pdg", aprime->getPdgID());
  ntuple_.setVar<int>("aprime_genstatus", aprime->getGenStatus());
  ntuple_.setVar<double>("aprime_mass", aprime->getMass());
  ntuple_.setVar<double>("aprime_energy", energy(aprime_p,aprime->getMass()));
  ntuple_.setVar<double>("aprime_px", aprime_p.at(0));
  ntuple_.setVar<double>("aprime_py", aprime_p.at(1));
  ntuple_.setVar<double>("aprime_pz", aprime_p.at(2));
  ntuple_.setVar<double>("beam_energy", beam->getEnergy());
  ntuple_.setVar<double>("visible_energy", visible_energy);
}

}

DECLARE_ANALYZER_NS(dqm,NtuplizeDarkBremInteraction);
