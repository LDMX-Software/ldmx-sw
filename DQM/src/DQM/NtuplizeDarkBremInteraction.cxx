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

void NtuplizeDarkBremInteraction::analyze(const framework::Event& e) {
  const auto& particle_map{e.getMap<int,ldmx::SimParticle>("SimParticles")};
  const ldmx::SimParticle *recoil{nullptr}, *aprime{nullptr}, *beam{nullptr}, *incident{nullptr};
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
        auto parent_id = particle.getParents().at(0);
        if (particle_map.find(parent_id) != particle_map.end()) {
          incident = &(particle_map.at(parent_id));
        }
      }
    }
  }

  if (recoil == nullptr or aprime == nullptr or beam == nullptr or incident == nullptr) {
    EXCEPTION_RAISE("BadEvent","Unable to find all necessary particles.");
  }

  static auto energy = [](const std::vector<double>& p, const double& m) {
    return sqrt(p.at(0)*p.at(0)+ p.at(1)*p.at(1)+ p.at(2)*p.at(2)+ m*m);
  };

  double incident_energy = energy(incident->getEndPointMomentum(), incident->getMass());
  double recoil_energy = energy(recoil->getMomentum(), recoil->getMass());
  double visible_energy = (beam->getEnergy() - incident_energy) + recoil_energy;

  ntuple_.setVar<double>("x", aprime->getVertex().at(0));
  ntuple_.setVar<double>("y", aprime->getVertex().at(1));
  ntuple_.setVar<double>("z", aprime->getVertex().at(2));
  ntuple_.setVar<double>("weight", e.getEventWeight());
  ntuple_.setVar<int>("incident_pdg", recoil->getPdgID());
  ntuple_.setVar<int>("incident_genstatus", -1);
  ntuple_.setVar<double>("incident_mass", recoil->getMass());
  ntuple_.setVar<double>("incident_energy", incident_energy);
  ntuple_.setVar<double>("incident_px", incident->getEndPointMomentum().at(0));
  ntuple_.setVar<double>("incident_py", incident->getEndPointMomentum().at(1));
  ntuple_.setVar<double>("incident_pz", incident->getEndPointMomentum().at(2));
  ntuple_.setVar<int>("recoil_pdg", recoil->getPdgID());
  ntuple_.setVar<int>("recoil_genstatus", recoil->getGenStatus());
  ntuple_.setVar<double>("recoil_mass", recoil->getMass());
  ntuple_.setVar<double>("recoil_energy", recoil_energy);
  ntuple_.setVar<double>("recoil_px", recoil->getMomentum().at(0));
  ntuple_.setVar<double>("recoil_py", recoil->getMomentum().at(1));
  ntuple_.setVar<double>("recoil_pz", recoil->getMomentum().at(2));
  ntuple_.setVar<int>("aprime_pdg", aprime->getPdgID());
  ntuple_.setVar<int>("aprime_genstatus", aprime->getGenStatus());
  ntuple_.setVar<double>("aprime_mass", aprime->getMass());
  ntuple_.setVar<double>("aprime_energy", energy(aprime->getMomentum(),aprime->getMass()));
  ntuple_.setVar<double>("aprime_px", aprime->getMomentum().at(0));
  ntuple_.setVar<double>("aprime_py", aprime->getMomentum().at(1));
  ntuple_.setVar<double>("aprime_pz", aprime->getMomentum().at(2));
  ntuple_.setVar<double>("beam_energy", beam->getEnergy());
  ntuple_.setVar<double>("visible_energy", visible_energy);
}

}

DECLARE_ANALYZER_NS(dqm,NtuplizeDarkBremInteraction);
