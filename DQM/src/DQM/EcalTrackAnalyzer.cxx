/**
 * @file EcalTrackAnalyzer.cxx
 * @brief Implementation of ECAL track analyzer
 */

#include "DQM/EcalTrackAnalyzer.h"

#include "Ecal/Event/EcalHit.h"

namespace dqm {

void EcalTrackAnalyzer::configure(framework::config::Parameters& ps) {
  track_collection_ = ps.get<std::string>("track_collection");
  track_pass_name_ = ps.get<std::string>("track_pass_name");
  rec_hit_collection_ = ps.get<std::string>("rec_hit_collection");
  rec_hit_pass_name_ = ps.get<std::string>("rec_hit_pass_name");
}

void EcalTrackAnalyzer::analyze(const framework::Event& event) {
  // Check if track collection exists
  if (!event.exists(track_collection_, track_pass_name_)) {
    ldmx_log(debug) << "ECAL track collection does not exist";
    return;
  }

  // Get ECAL tracks
  const auto& ecal_tracks =
      event.getCollection<ldmx::Track>(track_collection_, track_pass_name_);

  int n_tracks = ecal_tracks.size();
  histograms_.fill("n_tracks", n_tracks);

  if (n_tracks == 0) {
    return;
  }

  // Loop over tracks
  for (size_t i_track = 0; i_track < ecal_tracks.size(); ++i_track) {
    const auto& track = ecal_tracks[i_track];

    // Track quality
    double chi2 = track.getChi2();
    int ndf = track.getNdf();
    int nhits = track.getNhits();

    histograms_.fill("track_chi2", chi2);
    histograms_.fill("track_ndf", ndf);
    histograms_.fill("track_nhits", nhits);

    if (ndf > 0) {
      histograms_.fill("track_chi2_ndf", chi2 / ndf);
    }

    // Track parameters
    double d0 = track.getD0();
    double z0 = track.getZ0();
    double phi = track.getPhi();
    double theta = track.getTheta();
    double qop = track.getQoP();
    int charge = track.getCharge();

    histograms_.fill("track_d0", d0);
    histograms_.fill("track_z0", z0);
    histograms_.fill("track_phi", phi);
    histograms_.fill("track_theta", theta);
    histograms_.fill("track_qop", qop);
    histograms_.fill("track_charge", charge);

    // Momentum
    double p = 0;
    if (std::abs(qop) > 1e-9) {
      p = 1.0 / std::abs(qop);
    }
    histograms_.fill("track_p", p);

    // Momentum components (approximate from angles and magnitude)
    double px = p * std::sin(theta) * std::cos(phi);
    double py = p * std::sin(theta) * std::sin(phi);
    double pz = p * std::cos(theta);

    double pt = std::sqrt(px * px + py * py);
    histograms_.fill("track_px", px);
    histograms_.fill("track_py", py);
    histograms_.fill("track_pz", pz);
    histograms_.fill("track_pt", pt);

    // Position at ECAL front (from d0, z0)
    // d0 and z0 are in the perigee frame
    double x = -d0 * std::sin(phi);  // Approximate x position
    double y = d0 * std::cos(phi);   // Approximate y position

    histograms_.fill("track_x", x);
    histograms_.fill("track_y", y);
    histograms_.fill("track_xy", x, y);

    // 2D correlations
    histograms_.fill("track_nhits_vs_chi2", nhits, chi2);
    histograms_.fill("track_nhits_vs_chi2_ndf", nhits, (ndf > 0) ? chi2 / ndf : 0.0);
    histograms_.fill("track_chi2_vs_nhits", chi2, nhits);
    histograms_.fill("track_p_vs_chi2", p, chi2);
    histograms_.fill("track_p_vs_nhits", p, nhits);
    histograms_.fill("track_p_vs_theta", p, theta);

    // If multiple tracks, fill special histograms
    if (n_tracks > 1) {
      histograms_.fill("track_p_multitracks", p);

      // Calculate angular separation between tracks
      for (size_t j_track = i_track + 1; j_track < ecal_tracks.size();
           ++j_track) {
        const auto& other_track = ecal_tracks[j_track];

        double phi1 = track.getPhi();
        double theta1 = track.getTheta();
        double phi2 = other_track.getPhi();
        double theta2 = other_track.getTheta();

        // Direction vectors
        double dx1 = std::sin(theta1) * std::cos(phi1);
        double dy1 = std::sin(theta1) * std::sin(phi1);
        double dz1 = std::cos(theta1);

        double dx2 = std::sin(theta2) * std::cos(phi2);
        double dy2 = std::sin(theta2) * std::sin(phi2);
        double dz2 = std::cos(theta2);

        // Dot product
        double dot = dx1 * dx2 + dy1 * dy2 + dz1 * dz2;
        double angle = std::acos(std::max(-1.0, std::min(1.0, dot)));

        histograms_.fill("track_separation", angle);
      }
    }
  }

  ldmx_log(debug) << "Analyzed " << n_tracks << " ECAL tracks";
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::EcalTrackAnalyzer)
