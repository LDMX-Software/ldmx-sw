#include "DQM/EcalSPTrackCompare.h"

#include <cmath>

#include "DetDescr/SimSpecialID.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Tracking/Event/Track.h"

namespace dqm {

void EcalSPTrackCompare::configure(framework::config::Parameters& ps) {
  ecal_sp_coll_name_ = ps.get<std::string>("ecal_sp_coll_name");
  ecal_sp_pass_name_ = ps.get<std::string>("ecal_sp_pass_name");
  track_collection_ = ps.get<std::string>("track_collection");
  track_pass_name_ = ps.get<std::string>("track_pass_name");
}

void EcalSPTrackCompare::analyze(const framework::Event& event) {
  // --- Find the primary electron at ECal scoring plane (plane 31) ---
  if (!event.exists(ecal_sp_coll_name_, ecal_sp_pass_name_)) {
    ldmx_log(debug) << "SP collection " << ecal_sp_coll_name_ << " not found";
    return;
  }

  const auto& sp_hits = event.getCollection<ldmx::SimTrackerHit>(
      ecal_sp_coll_name_, ecal_sp_pass_name_);

  ldmx_log(debug) << "SP hits: " << sp_hits.size();

  // Find the highest-momentum forward-going electron at plane 31.
  // In dark brem events the recoil electron has trackID != 1.
  const ldmx::SimTrackerHit* sp_electron = nullptr;
  double best_pz = 0;
  for (const auto& hit : sp_hits) {
    ldmx::SimSpecialID hit_id(hit.getID());
    if (hit_id.plane() != 31) continue;
    if (hit.getPdgID() != 11) continue;
    const auto p = hit.getMomentum();
    if (p[2] <= 0) continue;
    if (p[2] > best_pz) {
      best_pz = p[2];
      sp_electron = &hit;
    }
  }

  if (!sp_electron) {
    ldmx_log(debug) << "No forward electron found at ECal SP";
    return;
  }

  // Extract SP electron kinematics
  const auto sp_pos = sp_electron->getPosition();
  const auto sp_mom = sp_electron->getMomentum();
  double sp_x = sp_pos[0];
  double sp_y = sp_pos[1];

  ldmx_log(debug) << "SP electron: trackID=" << sp_electron->getTrackID()
                  << " pos=(" << sp_x << ", " << sp_y << ") pz=" << best_pz;
  double sp_px = sp_mom[0];
  double sp_py = sp_mom[1];
  double sp_pz = sp_mom[2];
  double sp_p = std::sqrt(sp_px * sp_px + sp_py * sp_py + sp_pz * sp_pz);
  double sp_pt = std::sqrt(sp_px * sp_px + sp_py * sp_py);
  double sp_theta = std::atan2(sp_pt, sp_pz);
  double sp_phi = std::atan2(sp_py, sp_px);
  if (sp_phi < 0) sp_phi += 2.0 * M_PI;  // Shift to [0, 2π]

  histograms_.fill("sp_x", sp_x);
  histograms_.fill("sp_y", sp_y);
  histograms_.fill("sp_p", sp_p);
  histograms_.fill("sp_pt", sp_pt);
  histograms_.fill("sp_theta", sp_theta);
  histograms_.fill("sp_phi", sp_phi);

  // --- Get reconstructed tracks ---
  if (!event.exists(track_collection_, track_pass_name_)) {
    histograms_.fill("n_tracks", 0);
    histograms_.fill("has_match", 0);
    return;
  }

  const auto& tracks =
      event.getCollection<ldmx::Track>(track_collection_, track_pass_name_);

  int n_tracks = tracks.size();
  histograms_.fill("n_tracks", n_tracks);

  if (n_tracks == 0) {
    histograms_.fill("has_match", 0);
    return;
  }

  // --- Find closest track to SP electron ---
  int best_idx = -1;
  double best_dr = 1e9;

  for (int i = 0; i < n_tracks; ++i) {
    const auto& track = tracks[i];
    double trk_theta = track.getTheta();
    double trk_phi = track.getPhi();

    // Angular distance
    double dtheta = trk_theta - sp_theta;
    double dphi = trk_phi - sp_phi;
    // Wrap dphi to [-pi, pi]
    while (dphi > M_PI) dphi -= 2.0 * M_PI;
    while (dphi < -M_PI) dphi += 2.0 * M_PI;

    double dr = std::sqrt(dtheta * dtheta + dphi * dphi);
    if (dr < best_dr) {
      best_dr = dr;
      best_idx = i;
    }
  }

  histograms_.fill("has_match", 1);

  // --- Fill residuals for the closest track ---
  const auto& best = tracks[best_idx];

  // Get track parameters at perigee
  double trk_d0 = best.getD0();
  double trk_phi = best.getPhi();
  double trk_theta = best.getTheta();
  double trk_qop = best.getQoP();
  double trk_p = (std::abs(trk_qop) > 1e-9) ? 1.0 / std::abs(trk_qop) : 0.0;

  // Track position at perigee (reference point is at ECAL front face z)
  auto perigee = best.getPerigeeLocation();
  double trk_x = perigee[0] + (-trk_d0 * std::sin(trk_phi));
  double trk_y = perigee[1] + (trk_d0 * std::cos(trk_phi));

  ldmx_log(debug) << "Track perigee: (" << perigee[0] << ", " << perigee[1]
                  << ", " << perigee[2] << ")  d0=" << trk_d0;
  ldmx_log(debug) << "Track pos: (" << trk_x << ", " << trk_y
                  << ")  p=" << trk_p << " theta=" << trk_theta
                  << " phi=" << trk_phi;

  // Derived track quantities
  double trk_pt = trk_p * std::sin(trk_theta);

  // Residuals
  double dx = trk_x - sp_x;
  double dy = trk_y - sp_y;
  double dtheta = trk_theta - sp_theta;
  double dphi = trk_phi - sp_phi;
  while (dphi > M_PI) dphi -= 2.0 * M_PI;
  while (dphi < -M_PI) dphi += 2.0 * M_PI;
  double dp = trk_p - sp_p;
  double dpt = trk_pt - sp_pt;
  double dp_frac = (sp_p > 1.0) ? dp / sp_p : 0.0;

  // Track properties
  histograms_.fill("trk_x", trk_x);
  histograms_.fill("trk_y", trk_y);
  histograms_.fill("trk_p", trk_p);
  histograms_.fill("trk_pt", trk_pt);
  histograms_.fill("trk_theta", trk_theta);
  histograms_.fill("trk_phi", trk_phi);

  // Residuals
  histograms_.fill("delta_x", dx);
  histograms_.fill("delta_y", dy);
  histograms_.fill("delta_theta", dtheta);
  histograms_.fill("delta_phi", dphi);
  histograms_.fill("delta_p", dp);
  histograms_.fill("delta_pt", dpt);
  histograms_.fill("delta_p_frac", dp_frac);
  histograms_.fill("delta_r", std::sqrt(dx * dx + dy * dy));
  histograms_.fill("delta_angle", best_dr);

  // 2D correlations
  histograms_.fill("sp_x_vs_trk_x", sp_x, trk_x);
  histograms_.fill("sp_y_vs_trk_y", sp_y, trk_y);
  histograms_.fill("sp_p_vs_trk_p", sp_p, trk_p);
  histograms_.fill("sp_theta_vs_trk_theta", sp_theta, trk_theta);
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::EcalSPTrackCompare)
