#include "Hcal/VisiblesFeatureProducer.h"

// LDMX
#include "DetDescr/HcalID.h"
#include "DetDescr/SimSpecialID.h"
#include "Ecal/Event/EcalHit.h"
#include "Hcal/Event/HcalHit.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Tracking/Event/Track.h"

// C++
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numeric>

namespace hcal {

void VisiblesFeatureProducer::configure(
    framework::config::Parameters &parameters) {
  training_ = parameters.get<bool>("training");
  training_file_ = parameters.get<std::string>("training_file");

  beam_energy_mev_ = parameters.get<double>("beam_energy");

  // collection names
  hcal_rec_collection_ = parameters.get<std::string>("hcal_rec_coll_name");
  hcal_rec_pass_name_ = parameters.get<std::string>("hcal_rec_pass_name");

  ecal_rec_collection_ = parameters.get<std::string>("ecal_rec_coll_name");
  ecal_rec_pass_name_ = parameters.get<std::string>("ecal_rec_pass_name");

  recoil_from_tracking_ = parameters.get<bool>("recoil_from_tracking");
  track_collection_ = parameters.get<std::string>("track_collection");
  track_pass_name_ = parameters.get<std::string>("track_pass_name");

  sp_collection_ = parameters.get<std::string>("sp_coll_name");
  sp_pass_name_ = parameters.get<std::string>("sp_pass_name");
  sim_particles_pass_name_ =
      parameters.get<std::string>("sim_particles_pass_name");
}

bool VisiblesFeatureProducer::inList(std::vector<int> parents, int track_id) {
  bool in_list = false;
  for (const int &i : parents) {
    if (i == track_id) {
      in_list = true;
    }
  }
  return in_list;
}

void VisiblesFeatureProducer::analyze(const framework::Event &event) {
  std::vector<double> bdt_features;

  auto particle_map{event.getMap<int, ldmx::SimParticle>(
      "SimParticles", sim_particles_pass_name_)};

  // Get target scoring plane hits for recoil electron
  // Use this to calculate the projected photon line vector
  // This currently uses truth-level information, but it should be replaced
  // by reconstructed tracker information, when available
  std::vector<double> gamma_p(3);
  std::vector<double> gamma_x0(3);

  std::vector<double> recoil_p(3);
  bool found_recoil_e = false;

  if (recoil_from_tracking_) {
    auto recoil_tracks{
        event.getCollection<ldmx::Track>(track_collection_, track_pass_name_)};
    // Fill this in later when you know how to use it
    for (auto &track : recoil_tracks) {
      // need to figure out how to best isolate candidate electron track
      if (track.q() == 1 && track.getNhits() == 5) {
        gamma_x0 = track.getPosition();
        gamma_p[0] = -1. * track.getMomentum()[0];
        gamma_p[1] = -1. * track.getMomentum()[1];
        gamma_p[2] = beam_energy_mev_ - track.getMomentum()[2];
      }
    }
  } else {
    if (event.exists(sp_collection_, sp_pass_name_)) {
      std::vector<ldmx::SimTrackerHit> target_sp_hits =
          event.getCollection<ldmx::SimTrackerHit>(sp_collection_,
                                                   sp_pass_name_);
      bool found_rec = false;
      for (auto const &it : particle_map) {
        for (auto const &sphit : target_sp_hits) {
          if (sphit.getPosition()[2] > 0) {
            if (it.first == sphit.getTrackID()) {
              if (it.second.getPdgID() == 622) {
                std::vector<float> x0_float = sphit.getPosition();
                std::vector<double> x0_double(x0_float.begin(), x0_float.end());
                gamma_x0 = x0_double;
                gamma_p = sphit.getMomentum();
                found_rec = true;
              }
              if (it.second.getPdgID() == 11 &&
                  inList(it.second.getParents(), 0)) {
                if (!found_rec) {
                  std::vector<float> x0_float = sphit.getPosition();
                  std::vector<double> x0_double(x0_float.begin(),
                                                x0_float.end());
                  gamma_x0 = x0_double;
                  gamma_p[0] = -1. * sphit.getMomentum()[0];
                  gamma_p[1] = -1. * sphit.getMomentum()[1];
                  gamma_p[2] = beam_energy_mev_ - sphit.getMomentum()[2];
                  found_rec = true;
                }
                recoil_p = sphit.getMomentum();
                found_recoil_e = true;
              }
            }
          }
        }
      }
    }
  }

  double p_mag = 0.;
  if (found_recoil_e) {
    p_mag = std::sqrt(recoil_p[0] * recoil_p[0] + recoil_p[1] * recoil_p[1] +
                      recoil_p[2] * recoil_p[2]);
  }

  std::vector<ldmx::EcalHit> ecal_rec_hits = event.getCollection<ldmx::EcalHit>(
      ecal_rec_collection_, ecal_rec_pass_name_);
  std::vector<ldmx::HcalHit> hcal_rec_hits = event.getCollection<ldmx::HcalHit>(
      hcal_rec_collection_, hcal_rec_pass_name_);

  double ecal_energy = 0.;
  double hcal_energy = 0.;
  bool hcal_containment = true;

  for (const ldmx::EcalHit &hit : ecal_rec_hits) {
    if (hit.getEnergy() > 0.) {
      ecal_energy += hit.getEnergy();
    }
  }
  for (const ldmx::HcalHit &hit : hcal_rec_hits) {
    if (hit.getEnergy() > 0.) {
      ldmx::HcalID detID(hit.getID());
      if (detID.getSection() != 0) {
        continue;
      }
      if (detID.getLayerID() == 1 && hit.getPE() > 5) {
        hcal_containment = false;
      }
      hcal_energy += 12. * hit.getEnergy();
    }
  }

  if (ecal_energy < 3160 && hcal_energy > 4840 && hcal_containment &&
      p_mag < 2400) {
    // initialize all of the features
    int n_layers_hit = 0;
    double x_std = 0.;
    double y_std = 0.;
    double z_std = 0.;
    double x_mean = 0.;
    double y_mean = 0.;
    double r_mean = 0.;
    int iso_hits = 0;
    double iso_energy = 0.;
    int n_readout_hits = 0;
    double summed_det = 0.;
    double r_mean_from_photon_track = 0.;

    double z_mean = 0.;  // need this when calculating z_std
    std::vector<int> layers_hit;

    for (const ldmx::HcalHit &hit : hcal_rec_hits) {
      if (hit.getEnergy() > 0.) {
        ldmx::HcalID detID(hit.getID());
        if (detID.getSection() != 0) {  // skip hits that aren't in main Hcal
          continue;
        }
        if (fabs(hit.getXPos()) > 1000 || fabs(hit.getYPos()) > 1000) {
          continue;
        }

        n_readout_hits += 1;
        double x = hit.getXPos();
        double y = hit.getYPos();
        double z = hit.getZPos();
        double r = sqrt(pow(x, 2) + pow(y, 2));

        summed_det += hit.getEnergy();

        x_mean += x * hit.getEnergy();
        y_mean += y * hit.getEnergy();
        z_mean += z * hit.getEnergy();
        r_mean += r * hit.getEnergy();

        // check if this is a new layer in the collection
        if (!(std::find(layers_hit.begin(), layers_hit.end(),
                        detID.getLayerID()) != layers_hit.end())) {
          layers_hit.push_back(detID.getLayerID());
        }

        double x_proj =
            gamma_x0[0] + (z - gamma_x0[2]) * gamma_p[0] / gamma_p[2];
        double y_proj =
            gamma_x0[1] + (z - gamma_x0[2]) * gamma_p[1] / gamma_p[2];

        r_mean_from_photon_track +=
            hit.getEnergy() * sqrt(pow(x - x_proj, 2) + pow(y - y_proj, 2));

        // Calculate isolated hits
        double closest_point = 9999.;
        for (const ldmx::HcalHit &hit2 : hcal_rec_hits) {
          if (hit2.getEnergy() > 0.) {
            ldmx::HcalID detID2(hit2.getID());
            if (fabs(hit2.getXPos()) > 1000 || fabs(hit2.getYPos()) > 1000) {
              continue;
            }
            if (detID2.getLayerID() == detID.getLayerID()) {
              // Determine if a bar is vertical (along y-axis) or horizontal
              // (along x-axis) Odd layers have horizontal strips Even layers
              // have vertical strips
              if (detID2.getLayerID() % 2 == 0) {
                if (fabs(hit2.getYPos() - y) > 0) {
                  if (fabs(hit2.getYPos() - y) < closest_point) {
                    closest_point = fabs(hit2.getYPos() - y);
                  }
                }
              } else {
                if (fabs(hit2.getXPos() - x) > 0) {
                  if (fabs(hit2.getXPos() - x) < closest_point) {
                    closest_point = fabs(hit2.getXPos() - x);
                  }
                }
              }
            }
          }
        }
        if (closest_point > 50.) {
          iso_hits += 1;
          iso_energy += hit.getEnergy();
        }
      }
    }

    n_layers_hit = layers_hit.size();

    if (summed_det > 0.) {
      x_mean /= summed_det;
      y_mean /= summed_det;
      z_mean /= summed_det;
      r_mean /= summed_det;

      r_mean_from_photon_track /= summed_det;
    }

    for (const ldmx::HcalHit &hit : hcal_rec_hits) {
      if (hit.getEnergy() > 0.) {
        if (fabs(hit.getXPos()) > 1000 || fabs(hit.getYPos()) > 1000) {
          continue;
        }
        ldmx::HcalID detID(hit.getID());
        if (detID.getSection() == 0) {
          x_std += hit.getEnergy() * pow(hit.getXPos() - x_mean, 2);
          y_std += hit.getEnergy() * pow(hit.getYPos() - y_mean, 2);
          z_std += hit.getEnergy() * pow(hit.getZPos() - z_mean, 2);
        }
      }
    }

    if (summed_det > 0.) {
      x_std = sqrt(x_std / summed_det);
      y_std = sqrt(y_std / summed_det);
      z_std = sqrt(z_std / summed_det);
    }

    // Fill histograms
    histograms_.fill("layers_hit", n_layers_hit);
    histograms_.fill("x_std", x_std);
    histograms_.fill("y_std", y_std);
    histograms_.fill("z_std", z_std);
    histograms_.fill("x_mean", x_mean);
    histograms_.fill("y_mean", y_mean);
    histograms_.fill("r_mean", r_mean);
    histograms_.fill("iso_hits", iso_hits);
    histograms_.fill("iso_energy", iso_energy);
    histograms_.fill("n_hits", n_readout_hits);
    histograms_.fill("total_energy", hcal_energy);
    histograms_.fill("photon_track", r_mean_from_photon_track);

    bdt_features.push_back(n_layers_hit);
    bdt_features.push_back(x_std);
    bdt_features.push_back(y_std);
    bdt_features.push_back(z_std);
    bdt_features.push_back(x_mean);
    bdt_features.push_back(y_mean);
    bdt_features.push_back(r_mean);
    bdt_features.push_back(iso_hits);
    bdt_features.push_back(iso_energy);
    bdt_features.push_back(n_readout_hits);
    bdt_features.push_back(hcal_energy);
    bdt_features.push_back(r_mean_from_photon_track);

    if (training_) {
      std::ofstream file(training_file_, std::ios::app);
      if (!file.is_open()) {
        ldmx_log(fatal) << "Error: Could not open file " << training_file_;
        return;
      }
      for (int i = 0; i < bdt_features.size(); ++i) {
        file << bdt_features[i] << (i + 1 == bdt_features.size() ? "\n" : ", ");
      }
    }
  }

  return;
}

}  // namespace hcal

DECLARE_ANALYZER(hcal::VisiblesFeatureProducer);
