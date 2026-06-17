#include "Tracking/Reco/TrackComparisonProcessor.h"

#include <cmath>
#include <map>

#include "Framework/Logger.h"
#include "SimCore/Event/SimParticle.h"
#include "Tracking/Event/Track.h"

namespace tracking::reco {

TrackComparisonProcessor::TrackComparisonProcessor(const std::string& name,
                                                   framework::Process& process)
    : framework::Analyzer(name, process) {}

void TrackComparisonProcessor::configure(
    framework::config::Parameters& parameters) {
  trk_collection_smear_ =
      parameters.get<std::string>("trk_collection_smear", "TaggerTracks");
  trk_collection_digi_ =
      parameters.get<std::string>("trk_collection_digi", "TaggerDigiTracks");
  pass_name_smear_ = parameters.get<std::string>("pass_name_smear", "");
  pass_name_digi_ = parameters.get<std::string>("pass_name_digi", "");
  do_tagger_ = parameters.get<bool>("do_tagger", true);
  do_recoil_ = parameters.get<bool>("do_recoil", false);
  recoil_collection_smear_ =
      parameters.get<std::string>("recoil_collection_smear", "RecoilTracks");
  recoil_collection_digi_ =
      parameters.get<std::string>("recoil_collection_digi", "RecoilDigiTracks");
  recoil_pass_smear_ = parameters.get<std::string>("recoil_pass_smear", "");
  recoil_pass_digi_ = parameters.get<std::string>("recoil_pass_digi", "");
  min_truth_prob_ = parameters.get<double>("min_truth_prob", 0.5);
  sim_particles_pass_ = parameters.get<std::string>("sim_particles_pass", "");
  output_file_ =
      parameters.get<std::string>("output_file", "track_comparison.root");
}

void TrackComparisonProcessor::setupTree(TTree* tree, PairVars& v) {
  tree->Branch("track_id", &v.track_id_);
  tree->Branch("truth_prob_s", &v.truth_prob_s_);
  tree->Branch("truth_prob_d", &v.truth_prob_d_);
  tree->Branch("nhits_s", &v.nhits_s_);
  tree->Branch("nhits_d", &v.nhits_d_);
  tree->Branch("chi2ndf_s", &v.chi2ndf_s_);
  tree->Branch("chi2ndf_d", &v.chi2ndf_d_);
  tree->Branch("d0_s", &v.d0_s_);
  tree->Branch("d0_d", &v.d0_d_);
  tree->Branch("z0_s", &v.z0_s_);
  tree->Branch("z0_d", &v.z0_d_);
  tree->Branch("phi_s", &v.phi_s_);
  tree->Branch("phi_d", &v.phi_d_);
  tree->Branch("theta_s", &v.theta_s_);
  tree->Branch("theta_d", &v.theta_d_);
  tree->Branch("qop_s", &v.qop_s_);
  tree->Branch("qop_d", &v.qop_d_);
  tree->Branch("p_s", &v.p_s_);
  tree->Branch("p_d", &v.p_d_);
  tree->Branch("delta_d0", &v.delta_d0_);
  tree->Branch("delta_z0", &v.delta_z0_);
  tree->Branch("delta_phi", &v.delta_phi_);
  tree->Branch("delta_theta", &v.delta_theta_);
  tree->Branch("delta_p_over_p", &v.delta_p_over_p_);
  tree->Branch("px_s", &v.px_s_);
  tree->Branch("py_s", &v.py_s_);
  tree->Branch("pz_s", &v.pz_s_);
  tree->Branch("px_d", &v.px_d_);
  tree->Branch("py_d", &v.py_d_);
  tree->Branch("pz_d", &v.pz_d_);
  tree->Branch("px_t", &v.px_t_);
  tree->Branch("py_t", &v.py_t_);
  tree->Branch("pz_t", &v.pz_t_);
  tree->Branch("p_t", &v.p_t_);
  tree->Branch("vx_t", &v.vx_t_);
  tree->Branch("vy_t", &v.vy_t_);
  tree->Branch("vz_t", &v.vz_t_);
  tree->Branch("delta_p_over_p_s", &v.delta_p_over_p_s_);
  tree->Branch("delta_p_over_p_d", &v.delta_p_over_p_d_);
}

void TrackComparisonProcessor::onProcessStart() {
  file_ = new TFile(output_file_.c_str(), "RECREATE");
  file_->cd();

  if (do_tagger_) {
    tagger_tree_ =
        new TTree("tagger_pairs", "Tagger smear-vs-digi track pairs");
    tagger_tree_->SetDirectory(file_);
    setupTree(tagger_tree_, tagger_vars_);

    histograms_.create("tagger_delta_d0", "#Delta d_{0} (digi-smear) [mm]", 200,
                       -0.5, 0.5);
    histograms_.create("tagger_delta_z0", "#Delta z_{0} (digi-smear) [mm]", 200,
                       -2.0, 2.0);
    histograms_.create("tagger_delta_phi", "#Delta #phi (digi-smear) [rad]",
                       200, -0.02, 0.02);
    histograms_.create("tagger_delta_theta", "#Delta #theta (digi-smear) [rad]",
                       200, -0.02, 0.02);
    histograms_.create("tagger_delta_p_over_p", "#Delta p/p (digi-smear)/smear",
                       200, -0.1, 0.1);
    histograms_.create("tagger_nhits_s", "N hits (smear)", 15, 0, 15);
    histograms_.create("tagger_nhits_d", "N hits (digi)", 15, 0, 15);
    histograms_.create("tagger_chi2ndf_s", "#chi^{2}/ndf (smear)", 100, 0, 10);
    histograms_.create("tagger_chi2ndf_d", "#chi^{2}/ndf (digi)", 100, 0, 10);
    histograms_.create("tagger_p_s", "p (smear) [MeV]", 200, 0, 8000);
    histograms_.create("tagger_p_d", "p (digi) [MeV]", 200, 0, 8000);
    histograms_.create("tagger_p_t", "p (truth) [MeV]", 200, 0, 8000);
    histograms_.create("tagger_delta_p_over_p_s",
                       "#Delta p/p (smear-truth)/truth", 200, -0.1, 0.1);
    histograms_.create("tagger_delta_p_over_p_d",
                       "#Delta p/p (digi-truth)/truth", 200, -0.1, 0.1);
  }

  if (do_recoil_) {
    recoil_tree_ =
        new TTree("recoil_pairs", "Recoil smear-vs-digi track pairs");
    recoil_tree_->SetDirectory(file_);
    setupTree(recoil_tree_, recoil_vars_);

    histograms_.create("recoil_delta_d0", "#Delta d_{0} (digi-smear) [mm]", 200,
                       -2.0, 2.0);
    histograms_.create("recoil_delta_z0", "#Delta z_{0} (digi-smear) [mm]", 200,
                       -5.0, 5.0);
    histograms_.create("recoil_delta_phi", "#Delta #phi (digi-smear) [rad]",
                       200, -0.02, 0.02);
    histograms_.create("recoil_delta_theta", "#Delta #theta (digi-smear) [rad]",
                       200, -0.02, 0.02);
    histograms_.create("recoil_delta_p_over_p", "#Delta p/p (digi-smear)/smear",
                       200, -0.4, 0.4);
    histograms_.create("recoil_nhits_s", "N hits (smear)", 15, 0, 15);
    histograms_.create("recoil_nhits_d", "N hits (digi)", 15, 0, 15);
    histograms_.create("recoil_chi2ndf_s", "#chi^{2}/ndf (smear)", 100, 0, 10);
    histograms_.create("recoil_chi2ndf_d", "#chi^{2}/ndf (digi)", 100, 0, 10);
    histograms_.create("recoil_p_s", "p (smear) [MeV]", 200, 0, 8000);
    histograms_.create("recoil_p_d", "p (digi) [MeV]", 200, 0, 8000);
    histograms_.create("recoil_p_t", "p (truth) [MeV]", 200, 0, 8000);
    histograms_.create("recoil_delta_p_over_p_s",
                       "#Delta p/p (smear-truth)/truth", 200, -0.4, 0.4);
    histograms_.create("recoil_delta_p_over_p_d",
                       "#Delta p/p (digi-truth)/truth", 200, -0.4, 0.4);
  }
}

void TrackComparisonProcessor::fillPair(const ldmx::Track& smear,
                                        const ldmx::Track& digi,
                                        const ldmx::SimParticle& truth,
                                        PairVars& v,
                                        const std::string& prefix) {
  v.track_id_ = smear.getTrackID();
  v.truth_prob_s_ = smear.getTruthProb();
  v.truth_prob_d_ = digi.getTruthProb();
  v.nhits_s_ = smear.getNhits();
  v.nhits_d_ = digi.getNhits();
  v.chi2ndf_s_ = (smear.getNdf() > 0) ? smear.getChi2() / smear.getNdf() : -1;
  v.chi2ndf_d_ = (digi.getNdf() > 0) ? digi.getChi2() / digi.getNdf() : -1;
  v.d0_s_ = smear.getD0();
  v.d0_d_ = digi.getD0();
  v.z0_s_ = smear.getZ0();
  v.z0_d_ = digi.getZ0();
  v.phi_s_ = smear.getPhi();
  v.phi_d_ = digi.getPhi();
  v.theta_s_ = smear.getTheta();
  v.theta_d_ = digi.getTheta();
  v.qop_s_ = smear.getQoP();
  v.qop_d_ = digi.getQoP();
  // QoP is in e/GeV (ACTS units); convert to MeV so p_s/p_d match SimParticle
  // units.
  v.p_s_ = (v.qop_s_ != 0) ? std::abs(1000.0 / v.qop_s_) : -1;
  v.p_d_ = (v.qop_d_ != 0) ? std::abs(1000.0 / v.qop_d_) : -1;
  v.delta_d0_ = v.d0_d_ - v.d0_s_;
  v.delta_z0_ = v.z0_d_ - v.z0_s_;
  v.delta_phi_ = v.phi_d_ - v.phi_s_;
  v.delta_theta_ = v.theta_d_ - v.theta_s_;
  v.delta_p_over_p_ = (v.p_s_ > 0) ? (v.p_d_ - v.p_s_) / v.p_s_ : -999;

  auto mom_s = smear.getMomentumAtTarget();
  if (mom_s.size() == 3) {
    v.px_s_ = mom_s[0];
    v.py_s_ = mom_s[1];
    v.pz_s_ = mom_s[2];
  }
  auto mom_d = digi.getMomentumAtTarget();
  if (mom_d.size() == 3) {
    v.px_d_ = mom_d[0];
    v.py_d_ = mom_d[1];
    v.pz_d_ = mom_d[2];
  }

  auto mom_t = truth.getMomentum();
  v.px_t_ = mom_t[0];
  v.py_t_ = mom_t[1];
  v.pz_t_ = mom_t[2];
  v.p_t_ = std::sqrt(v.px_t_ * v.px_t_ + v.py_t_ * v.py_t_ + v.pz_t_ * v.pz_t_);
  auto vtx = truth.getVertex();
  v.vx_t_ = vtx[0];
  v.vy_t_ = vtx[1];
  v.vz_t_ = vtx[2];

  v.delta_p_over_p_s_ = (v.p_t_ > 0) ? (v.p_s_ - v.p_t_) / v.p_t_ : -999;
  v.delta_p_over_p_d_ = (v.p_t_ > 0) ? (v.p_d_ - v.p_t_) / v.p_t_ : -999;

  histograms_.fill(prefix + "delta_d0", v.delta_d0_);
  histograms_.fill(prefix + "delta_z0", v.delta_z0_);
  histograms_.fill(prefix + "delta_phi", v.delta_phi_);
  histograms_.fill(prefix + "delta_theta", v.delta_theta_);
  histograms_.fill(prefix + "delta_p_over_p", v.delta_p_over_p_);
  histograms_.fill(prefix + "nhits_s", v.nhits_s_);
  histograms_.fill(prefix + "nhits_d", v.nhits_d_);
  histograms_.fill(prefix + "chi2ndf_s", v.chi2ndf_s_);
  histograms_.fill(prefix + "chi2ndf_d", v.chi2ndf_d_);
  histograms_.fill(prefix + "p_s", v.p_s_);
  histograms_.fill(prefix + "p_d", v.p_d_);
  histograms_.fill(prefix + "p_t", v.p_t_);
  histograms_.fill(prefix + "delta_p_over_p_s", v.delta_p_over_p_s_);
  histograms_.fill(prefix + "delta_p_over_p_d", v.delta_p_over_p_d_);
}

void TrackComparisonProcessor::processTracker(const framework::Event& event,
                                              const std::string& coll_smear,
                                              const std::string& pass_smear,
                                              const std::string& coll_digi,
                                              const std::string& pass_digi,
                                              TTree* tree, PairVars& vars,
                                              const std::string& histo_prefix) {
  if (!event.exists(coll_smear, pass_smear)) {
    ldmx_log(warn) << "Smear collection " << coll_smear << " not found";
    return;
  }
  if (!event.exists(coll_digi, pass_digi)) {
    ldmx_log(warn) << "Digi collection " << coll_digi << " not found";
    return;
  }

  const auto& tracks_smear =
      event.getCollection<ldmx::Track>(coll_smear, pass_smear);
  const auto& tracks_digi =
      event.getCollection<ldmx::Track>(coll_digi, pass_digi);

  const auto& particle_map =
      event.getMap<int, ldmx::SimParticle>("SimParticles", sim_particles_pass_);

  std::map<int, const ldmx::Track*> smear_by_id;
  for (const auto& t : tracks_smear) {
    if (t.getTruthProb() >= min_truth_prob_) smear_by_id[t.getTrackID()] = &t;
  }

  for (const auto& t : tracks_digi) {
    if (t.getTruthProb() < min_truth_prob_) continue;
    auto it = smear_by_id.find(t.getTrackID());
    if (it == smear_by_id.end()) continue;
    auto pit = particle_map.find(t.getTrackID());
    if (pit == particle_map.end()) continue;

    fillPair(*it->second, t, pit->second, vars, histo_prefix);
    tree->Fill();
  }
}

void TrackComparisonProcessor::analyze(const framework::Event& event) {
  if (do_tagger_) {
    processTracker(event, trk_collection_smear_, pass_name_smear_,
                   trk_collection_digi_, pass_name_digi_, tagger_tree_,
                   tagger_vars_, "tagger_");
  }
  if (do_recoil_) {
    processTracker(event, recoil_collection_smear_, recoil_pass_smear_,
                   recoil_collection_digi_, recoil_pass_digi_, recoil_tree_,
                   recoil_vars_, "recoil_");
  }
}

void TrackComparisonProcessor::onProcessEnd() {
  file_->cd();
  if (tagger_tree_) tagger_tree_->Write("", TObject::kOverwrite);
  if (recoil_tree_) recoil_tree_->Write("", TObject::kOverwrite);
  file_->Close();
}

}  // namespace tracking::reco

DECLARE_ANALYZER(tracking::reco::TrackComparisonProcessor)
