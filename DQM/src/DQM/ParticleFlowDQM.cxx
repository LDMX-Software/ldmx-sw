#include "DQM/ParticleFlowDQM.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <vector>

#include "Recon/Event/PFCandidate.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"

namespace dqm {

namespace {

// bins shared by the reco and truth type axes, see dqm.py; charged up to kP
enum Category { kE, kMu, kPi, kK, kP, kGamma, kNHad, kOther, kNone };

int category(int pdg) {
  switch (std::abs(pdg)) {
    case 11:
      return kE;
    case 13:
      return kMu;
    case 211:
      return kPi;
    case 321:
      return kK;
    case 2212:
      return kP;
    case 22:
      return kGamma;
    case 2112:
    case 130:
    case 310:
    case 3122:
      return kNHad;
    default:
      return kOther;
  }
}

// a particle crossing the Ecal front face
struct TruthParticle {
  int track_id_;
  int pdg_;
  double visible_e_;  // total for e and gamma, kinetic otherwise
  double total_e_;
  double x_, y_, z_, px_, py_, pz_;
  bool matched_{false};
};

// truth position projected along the momentum to z
std::pair<double, double> project(const TruthParticle& t, double z) {
  if (t.pz_ <= 0) return {t.x_, t.y_};
  return {t.x_ + t.px_ / t.pz_ * (z - t.z_), t.y_ + t.py_ / t.pz_ * (z - t.z_)};
}

// candidate direction from the target, unit vector
std::vector<double> direction(const ldmx::PFCandidate& c) {
  auto xyz = (c.getPID() & 2) ? c.getEcalClusterXYZ() : c.getHcalClusterXYZ();
  double n = std::hypot(xyz[0], xyz[1], xyz[2]);
  if (n <= 0) return {0, 0, 1};
  return {xyz[0] / n, xyz[1] / n, xyz[2] / n};
}

}  // namespace

void ParticleFlowDQM::configure(framework::config::Parameters& ps) {
  pf_cand_coll_name_ = ps.get<std::string>("pf_cand_coll_name");
  pf_cand_pass_name_ = ps.get<std::string>("pf_cand_pass_name");
  ecal_sp_coll_name_ = ps.get<std::string>("ecal_sp_coll_name");
  ecal_sp_pass_name_ = ps.get<std::string>("ecal_sp_pass_name");
  sim_particles_coll_name_ = ps.get<std::string>("sim_particles_coll_name");
  sim_particles_pass_name_ = ps.get<std::string>("sim_particles_pass_name");
  truth_min_energy_ = ps.get<double>("truth_min_energy");
  neutral_match_dist_ = ps.get<double>("neutral_match_dist");
  neutral_match_max_ratio_ = ps.get<double>("neutral_match_max_ratio");
}

void ParticleFlowDQM::analyze(const framework::Event& event) {
  if (!event.exists(pf_cand_coll_name_, pf_cand_pass_name_)) {
    ldmx_log(debug) << "No collection " << pf_cand_coll_name_;
    return;
  }
  auto cands = event.getCollection<ldmx::PFCandidate>(pf_cand_coll_name_,
                                                      pf_cand_pass_name_);
  histograms_.fill("n_cands", cands.size());

  int n_charged = 0, n_neutral = 0;
  double charged_p = 0, neutral_e = 0, pf_e = 0;
  for (auto& cand : cands) {
    int pid = cand.getPID();
    histograms_.fill("pid", pid);
    histograms_.fill("reco_type", category(cand.getPdgId()));
    pf_e += cand.getEnergy();
    double ecal_e = cand.getEcalEnergy();
    double hcal_e = cand.getHcalEnergy();

    if (pid & 1) {
      n_charged++;
      auto p = cand.getTrackPxPyPz();
      double trk_p = std::hypot(p[0], p[1], p[2]);
      charged_p += trk_p;
      histograms_.fill("charged_track_p", trk_p);
      histograms_.fill("charged_is_ecal_linked", (pid & 2) ? 1 : 0);
      if (cand.getDeDxNhits() > 0) {
        histograms_.fill("charged_dedx_mass2_vs_p", trk_p,
                         cand.getDeDxMassSquared());
      }
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
        int t_cat = category(cand.getTruthPdgId());
        if ((pid & 2) && trk_p > 0) {
          histograms_.fill("charged_e_over_p_vs_truth", t_cat, ecal_e / trk_p);
        }
        if (cand.getDeDxNhits() > 0) {
          histograms_.fill("charged_dedx_mass2_vs_truth", t_cat,
                           cand.getDeDxMassSquared());
        }
      }
    } else {
      n_neutral++;
      neutral_e += cand.getEnergy();
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
  histograms_.fill("sum_pf_e", pf_e);

  // reco diphoton mass, directions from the target
  std::vector<int> photons;
  for (int i = 0; i < static_cast<int>(cands.size()); i++) {
    if (cands[i].getPdgId() == 22) photons.push_back(i);
  }
  for (size_t a = 0; a < photons.size(); a++) {
    for (size_t b = a + 1; b < photons.size(); b++) {
      const auto& c1 = cands[photons[a]];
      const auto& c2 = cands[photons[b]];
      auto d1 = direction(c1);
      auto d2 = direction(c2);
      double cos12 = d1[0] * d2[0] + d1[1] * d2[1] + d1[2] * d2[2];
      double m2 = 2 * c1.getEnergy() * c2.getEnergy() * (1 - cos12);
      histograms_.fill("mgg", std::sqrt(std::max(0., m2)));
    }
  }

  analyzeTruth(event, cands);
}

void ParticleFlowDQM::analyzeTruth(const framework::Event& event,
                                   std::vector<ldmx::PFCandidate>& cands) {
  if (!event.exists(ecal_sp_coll_name_, ecal_sp_pass_name_)) return;
  const auto& sp_hits = event.getCollection<ldmx::SimTrackerHit>(
      ecal_sp_coll_name_, ecal_sp_pass_name_);

  // particles entering the Ecal front face, highest energy per track
  std::map<int, TruthParticle> by_id;
  for (const auto& sp : sp_hits) {
    auto pos = sp.getPosition();
    auto mom = sp.getMomentum();
    if (std::abs(240 - pos[2]) > 0.1 || mom[2] <= 0) continue;
    double e = sp.getEnergy();
    int apdg = std::abs(sp.getPdgID());
    double vis = (apdg == 11 || apdg == 22)
                     ? e
                     : e - std::sqrt(std::max(0., e * e - mom[0] * mom[0] -
                                                      mom[1] * mom[1] -
                                                      mom[2] * mom[2]));
    if (vis < truth_min_energy_) continue;
    auto it = by_id.find(sp.getTrackID());
    if (it != by_id.end() && it->second.visible_e_ >= vis) continue;
    by_id[sp.getTrackID()] = {sp.getTrackID(), sp.getPdgID(), vis,    e,
                              pos[0],          pos[1],        pos[2], mom[0],
                              mom[1],          mom[2]};
  }
  std::vector<TruthParticle> truth;
  double truth_e = 0;
  // same convention as PF: total for charged, visible for neutral
  for (auto& [id, t] : by_id) {
    truth.push_back(t);
    int cat = category(t.pdg_);
    truth_e += (cat <= kP) ? t.total_e_ : t.visible_e_;
  }
  histograms_.fill("sum_truth_e", truth_e);
  double pf_e = 0;
  for (const auto& c : cands) pf_e += c.getEnergy();
  if (truth_e > 0) histograms_.fill("sum_pf_e_over_truth", pf_e / truth_e);

  // charged candidates match by track ID
  std::vector<int> cand_truth(cands.size(), -1);
  for (size_t i = 0; i < cands.size(); i++) {
    if (!(cands[i].getPID() & 1)) continue;
    for (size_t j = 0; j < truth.size(); j++) {
      if (!truth[j].matched_ &&
          truth[j].track_id_ == cands[i].getTruthTrackId()) {
        truth[j].matched_ = true;
        cand_truth[i] = j;
        break;
      }
    }
  }

  // neutral candidates match by position, highest energy first
  std::vector<size_t> neutral;
  for (size_t i = 0; i < cands.size(); i++) {
    if (!(cands[i].getPID() & 1)) neutral.push_back(i);
  }
  std::sort(neutral.begin(), neutral.end(), [&cands](size_t a, size_t b) {
    return cands[a].getEnergy() > cands[b].getEnergy();
  });
  for (size_t i : neutral) {
    auto xyz = (cands[i].getPID() & 2) ? cands[i].getEcalClusterXYZ()
                                       : cands[i].getHcalClusterXYZ();
    // Hcal clusters are wider, scale the window
    double max_d = neutral_match_dist_ * ((cands[i].getPID() & 2) ? 1. : 3.);
    int best = -1;
    double best_d = max_d;
    for (size_t j = 0; j < truth.size(); j++) {
      if (truth[j].matched_) continue;
      // fragments must not count as found particles
      double ratio = cands[i].getEnergy() / truth[j].visible_e_;
      if (ratio < 1. / neutral_match_max_ratio_ ||
          ratio > neutral_match_max_ratio_)
        continue;
      auto [tx, ty] = project(truth[j], xyz[2]);
      double d = std::hypot(tx - xyz[0], ty - xyz[1]);
      if (d < best_d) {
        best_d = d;
        best = j;
      }
    }
    if (best >= 0) {
      truth[best].matched_ = true;
      cand_truth[i] = best;
    }
  }

  // type confusion and energy response
  for (size_t i = 0; i < cands.size(); i++) {
    int r_cat = category(cands[i].getPdgId());
    if (cand_truth[i] < 0) {
      histograms_.fill("type_confusion", kNone, r_cat);
      if (!(cands[i].getPID() & 1)) {
        histograms_.fill("unmatched_neutral_e", cands[i].getEnergy());
      }
      continue;
    }
    const auto& t = truth[cand_truth[i]];
    histograms_.fill("type_confusion", category(t.pdg_), r_cat);
    // charged energy includes the mass, calorimeters see the visible part
    double ref = (cands[i].getPID() & 1) ? t.total_e_ : t.visible_e_;
    if (ref > 0) {
      histograms_.fill("response_vs_type", r_cat, cands[i].getEnergy() / ref);
    }
  }
  for (const auto& t : truth) {
    histograms_.fill("truth_type", category(t.pdg_));
    if (t.matched_) histograms_.fill("truth_type_matched", category(t.pdg_));
    if (t.pdg_ == 22) {
      histograms_.fill("truth_gamma_e", t.visible_e_);
      if (t.matched_) histograms_.fill("truth_gamma_e_matched", t.visible_e_);
    }
  }

  // diphoton mass of photon pairs from the same pi0
  if (!event.exists(sim_particles_coll_name_, sim_particles_pass_name_)) return;
  const auto& particles = event.getMap<int, ldmx::SimParticle>(
      sim_particles_coll_name_, sim_particles_pass_name_);
  auto pi0_parent = [&particles](int track_id) {
    auto it = particles.find(track_id);
    if (it == particles.end() || it->second.getParents().empty()) return -1;
    int parent = it->second.getParents()[0];
    auto pit = particles.find(parent);
    if (pit == particles.end() || pit->second.getPdgID() != 111) return -1;
    return parent;
  };
  for (size_t a = 0; a < cands.size(); a++) {
    if (cands[a].getPdgId() != 22 || cand_truth[a] < 0) continue;
    int pa = pi0_parent(truth[cand_truth[a]].track_id_);
    if (pa < 0) continue;
    for (size_t b = a + 1; b < cands.size(); b++) {
      if (cands[b].getPdgId() != 22 || cand_truth[b] < 0) continue;
      if (pi0_parent(truth[cand_truth[b]].track_id_) != pa) continue;
      auto d1 = direction(cands[a]);
      auto d2 = direction(cands[b]);
      double cos12 = d1[0] * d2[0] + d1[1] * d2[1] + d1[2] * d2[2];
      double m2 = 2 * cands[a].getEnergy() * cands[b].getEnergy() * (1 - cos12);
      histograms_.fill("mgg_pi0_matched", std::sqrt(std::max(0., m2)));
    }
  }
  // truth pi0 photons entering the Ecal, for the pair efficiency
  std::map<int, int> n_photons;
  for (const auto& t : truth) {
    if (t.pdg_ != 22) continue;
    int pa = pi0_parent(t.track_id_);
    if (pa >= 0) n_photons[pa]++;
  }
  for (const auto& [pa, n] : n_photons) {
    histograms_.fill("truth_pi0_photons_in_ecal", n);
  }
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::ParticleFlowDQM)
