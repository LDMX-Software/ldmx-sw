// LDMX
#include "Recon/TrackDeDxMassEstimator.h"

#include "Recon/Event/TrackDeDxMassEstimate.h"

// STL
#include <algorithm>  // for std::transform
#include <cctype>     // for ::tolower
#include <iostream>

namespace recon {

void TrackDeDxMassEstimator::configure(framework::config::Parameters& ps) {
  fit_res_c_ = ps.get<double>("fit_res_c");
  fit_res_k_ = ps.get<double>("fit_res_k");
  input_pass_name_ = ps.get<std::string>("input_pass_name");
  track_collection_ = ps.get<std::string>("track_collection");

  ldmx_log(info) << "Track Collection used for TrackDeDxMassEstimator "
                 << track_collection_;
}

void TrackDeDxMassEstimator::produce(framework::Event& event) {
  std::vector<ldmx::TrackDeDxMassEstimate> mass_estimates;

  if (!event.exists(track_collection_, input_pass_name_)) {
    ldmx_log(error) << "Track collection " << track_collection_ << "_"
                    << input_pass_name_ << " not in event, exiting...";
    event.add("TrackDeDxMassEstimate", mass_estimates);
    return;
  }
  const std::vector<ldmx::Track> tracks{
      event.getCollection<ldmx::Track>(track_collection_, input_pass_name_)};

  int track_type;
  std::string track_coll_str = track_collection_;
  std::transform(track_coll_str.begin(), track_coll_str.end(),
                 track_coll_str.begin(), ::tolower);

  bool is_truth = track_coll_str.find("truth") != std::string::npos;

  if (is_truth) {
    if (track_coll_str.find("tagger") != std::string::npos) {
      track_type = 1;
      simhit_collection_ = "TaggerSimHits";
    } else if (track_coll_str.find("recoil") != std::string::npos) {
      track_type = 2;
      simhit_collection_ = "RecoilSimHits";
    } else {
      track_type = 0;
      simhit_collection_ = "";
    }
  } else {
    // Reco tracks (e.g. RecoilTracks, RecoilTracksClean)
    track_type = 4;
    simhit_collection_ = "";
  }

  // Retrieve simhits only for truth tracks
  std::vector<ldmx::SimTrackerHit> simhits;
  if (is_truth) {
    if (!event.exists(simhit_collection_, input_pass_name_)) {
      ldmx_log(error) << " SimHit collection (" << simhit_collection_ << "_"
                      << input_pass_name_ << ") does not exists, exiting...";
      event.add("TrackDeDxMassEstimate", mass_estimates);
      return;
    }
    simhits = event.getCollection<ldmx::SimTrackerHit>(simhit_collection_,
                                                       input_pass_name_);
  }

  // Loop over the collection of tracks
  for (uint i = 0; i < tracks.size(); i++) {
    auto track = tracks.at(i);
    // If track momentum doen't exist, skip
    auto the_qop = track.getQoP();
    if (the_qop == 0) {
      ldmx_log(debug) << "Track " << i << "has zero q/p ";
      continue;
    }

    int pdg_id = track.getPdgID();
    float momentum = 1. / std::abs(the_qop) * 1000;  // unit: MeV
    ldmx_log(debug) << "Track " << i << " has momentum " << momentum;

    ldmx::TrackDeDxMassEstimate mass_est;
    float sum_dedx_inv2 = 0.;
    float dedx;
    int n_hits = 0;

    if (is_truth) {
      // Use simhits associated with the truth track
      for (auto hit : simhits) {
        if (hit.getTrackID() != track.getTrackID()) continue;
        if (hit.getEdep() >= 0 && hit.getPathLength() > 0) {
          dedx = hit.getEdep() / hit.getPathLength() * 10;  // unit: MeV/cm
          sum_dedx_inv2 += 1. / (dedx * dedx);
          n_hits++;
        }
      }
    } else {
      // Use dE/dx measurements stored on the reco track (in MeV/mm)
      for (auto dedx_meas : track.getDedxMeasurements()) {
        if (dedx_meas > 0) {
          dedx = dedx_meas * 10;  // convert MeV/mm to MeV/cm
          sum_dedx_inv2 += 1. / (dedx * dedx);
          n_hits++;
        }
      }
    }  // end of loop over measurements

    if (sum_dedx_inv2 == 0) {
      ldmx_log(debug) << "Track " << i << " has no dEdx measurements";
      continue;
    }

    // Ih = (1/N * sum_i^N(dE/dx_i)^-2)^-1/2
    float the_ih = 1. / sqrt(1. / n_hits * sum_dedx_inv2);

    float mass = 0.;
    if (the_ih > fit_res_c_) {
      mass = momentum * sqrt((the_ih - fit_res_c_) / fit_res_k_);
    } else {
      ldmx_log(info) << "Track " << i << " has Ih " << the_ih
                     << " which is less than fit_res_C " << fit_res_c_;
      mass = -100.;
    }

    mass_est.setMomentum(momentum);
    mass_est.setNhits(n_hits);
    mass_est.setIh(the_ih);
    mass_est.setMass(mass);
    mass_est.setTrackIndex(i);
    mass_est.setTrackType(track_type);
    mass_est.setPdgId(pdg_id);
    mass_estimates.push_back(mass_est);
  }

  // Add the mass estimates to the event
  event.add("TrackDeDxMassEstimate", mass_estimates);
}
}  // namespace recon

DECLARE_PRODUCER(recon::TrackDeDxMassEstimator)
