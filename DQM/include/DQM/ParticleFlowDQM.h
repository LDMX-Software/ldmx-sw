#ifndef DQM_PARTICLEFLOWDQM_H
#define DQM_PARTICLEFLOWDQM_H

#include <string>
#include <vector>

#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Recon/Event/PFCandidate.h"

namespace dqm {

/**
 * @class ParticleFlowDQM
 * @brief Monitor PF candidates, their ID, and the match to truth
 *
 * Truth is every particle entering the Ecal front face. Charged
 * candidates match by track ID, neutral ones by position.
 */
class ParticleFlowDQM : public framework::Analyzer {
 public:
  ParticleFlowDQM(const std::string& name, framework::Process& process)
      : Analyzer(name, process) {}
  ~ParticleFlowDQM() override = default;

  void configure(framework::config::Parameters& ps) override;
  void analyze(const framework::Event& event) override;

 private:
  void analyzeTruth(const framework::Event& event,
                    std::vector<ldmx::PFCandidate>& cands);
  bool reconstructable(int pdg, double px, double py, double pz,
                       double ke) const;

  std::string pf_cand_coll_name_{"PFCandidates"};
  std::string pf_cand_pass_name_{""};
  std::string ecal_sp_coll_name_{"EcalScoringPlaneHits"};
  std::string ecal_sp_pass_name_{""};
  std::string sim_particles_coll_name_{"SimParticles"};
  std::string sim_particles_pass_name_{""};
  // minimum visible energy of a truth particle [MeV]
  double truth_min_energy_{50.};
  // neutral candidate to truth match window at the Ecal [mm]
  double neutral_match_dist_{30.};
  // max energy ratio either way for a neutral match
  double neutral_match_max_ratio_{2.};
  // Hcal only candidate to truth match cone from the target [deg]
  double neutral_hcal_match_angle_{30.};
  // reconstructable truth: max angle to the beam axis [deg], min |p| for
  // charged pi and for p, K [MeV], min KE for neutral hadrons and pi0 [MeV]
  double reco_max_theta_{80.};
  double reco_min_p_pion_{100.};
  double reco_min_p_heavy_{800.};
  double reco_min_ke_neutral_{1000.};
  double reco_min_ke_pi0_{2000.};
};

}  // namespace dqm

#endif
