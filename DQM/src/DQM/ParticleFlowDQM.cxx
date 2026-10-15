#include "DQM/ParticleFlowDQM.h"

#include <cmath>

#include "Recon/Event/PFCandidate.h"

namespace dqm {

void ParticleFlowDQM::configure(framework::config::Parameters& ps) {
  pf_cand_coll_name_ = ps.get<std::string>("pf_cand_coll_name");
  pf_cand_pass_name_ = ps.get<std::string>("pf_cand_pass_name");
}

void ParticleFlowDQM::analyze(const framework::Event& event) {
  if (!event.exists(pf_cand_coll_name_, pf_cand_pass_name_)) {
    ldmx_log(debug) << "No collection " << pf_cand_coll_name_;
    return;
  }
  const auto& cands = event.getCollection<ldmx::PFCandidate>(
      pf_cand_coll_name_, pf_cand_pass_name_);
  histograms_.fill("n_cands", cands.size());

  int n_charged = 0, n_neutral = 0;
  double charged_p = 0, neutral_e = 0;
  for (auto cand : cands) {  // copy, truth getters are non-const
    int pid = cand.getPID();
    histograms_.fill("pid", pid);
    double ecal_e = cand.getEcalEnergy();
    double hcal_e = cand.getHcalEnergy();

    if (pid & 1) {
      n_charged++;
      auto p = cand.getTrackPxPyPz();
      double trk_p = std::hypot(p[0], p[1], p[2]);
      charged_p += trk_p;
      histograms_.fill("charged_track_p", trk_p);
      histograms_.fill("charged_is_ecal_linked", (pid & 2) ? 1 : 0);
      if ((pid & 2) && trk_p > 0) {
        histograms_.fill("charged_ecal_e_over_p", ecal_e / trk_p);
        histograms_.fill("charged_ecal_raw_e_over_p",
                         cand.getEcalRawEnergy() / trk_p);
        // track at Ecal face vs linked cluster centroid
        auto trk_pos = cand.getEcalPositionXYZ();
        auto cl_pos = cand.getEcalClusterXYZ();
        histograms_.fill("charged_cluster_dx", cl_pos[0] - trk_pos[0]);
        histograms_.fill("charged_cluster_dy", cl_pos[1] - trk_pos[1]);
      }
      if (cand.getTruthPdgId() != 0) {
        histograms_.fill("charged_truth_pdg", cand.getTruthPdgId());
        auto tp = cand.getTruthPxPyPz();
        double true_p = std::hypot(tp[0], tp[1], tp[2]);
        if (true_p > 0) {
          histograms_.fill("charged_dp_over_p", (trk_p - true_p) / true_p);
          histograms_.fill("charged_p_vs_truth_p", true_p, trk_p);
        }
        auto t_pos = cand.getTruthEcalXYZ();
        auto r_pos = cand.getEcalPositionXYZ();
        histograms_.fill("charged_dx_truth", r_pos[0] - t_pos[0]);
        histograms_.fill("charged_dy_truth", r_pos[1] - t_pos[1]);
      }
    } else {
      n_neutral++;
      neutral_e += ecal_e + hcal_e;
      if (pid == 2) histograms_.fill("neutral_ecal_only_e", ecal_e);
      if (pid == 6) {
        histograms_.fill("neutral_ecal_hcal_ecal_e", ecal_e);
        histograms_.fill("neutral_ecal_hcal_hcal_e", hcal_e);
      }
      if (pid == 4) histograms_.fill("neutral_hcal_only_e", hcal_e);
    }
  }
  histograms_.fill("n_charged", n_charged);
  histograms_.fill("n_neutral", n_neutral);
  histograms_.fill("sum_charged_p", charged_p);
  histograms_.fill("sum_neutral_e", neutral_e);
  histograms_.fill("sum_pf_e", charged_p + neutral_e);
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::ParticleFlowDQM)
