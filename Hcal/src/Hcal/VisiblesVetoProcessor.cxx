#include "Hcal/VisiblesVetoProcessor.h"

// LDMX
#include "DetDescr/HcalID.h"
#include "DetDescr/SimSpecialID.h"
#include "Hcal/Event/HcalHit.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"

// C++
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numeric>

namespace hcal {
void VisiblesVetoProcessor::buildBDTFeatureVector(
    const ldmx::VisiblesVetoResult &result) {
  bdt_features_.push_back(result.getNLayersHit());
  bdt_features_.push_back(result.getXStd());
  bdt_features_.push_back(result.getYStd());
  bdt_features_.push_back(result.getZStd());
  bdt_features_.push_back(result.getXMean());
  bdt_features_.push_back(result.getYMean());
  bdt_features_.push_back(result.getRMean());
  bdt_features_.push_back(result.getIsoHits());
  bdt_features_.push_back(result.getIsoEnergy());
  bdt_features_.push_back(result.getNReadoutHits());
  bdt_features_.push_back(result.getSummedDet());
  bdt_features_.push_back(result.getDistFromPhotonTrack());
}

void VisiblesVetoProcessor::configure(
    framework::config::Parameters &parameters) {
  feature_list_name_ = parameters.get<std::string>("feature_list_name");
  // Load BDT ONNX file
  rt_ = std::make_unique<ldmx::ort::ONNXRuntime>(
      parameters.get<std::string>("bdt_file"));

  bdt_cut_val_ = parameters.get<double>("disc_cut");

  beam_energy_mev_ = parameters.get<double>("beam_energy");

  // collection and pass names
  collection_name_ = parameters.get<std::string>("collection_name");
  rec_pass_name_ = parameters.get<std::string>("rec_pass_name");
  rec_coll_name_ = parameters.get<std::string>("rec_coll_name");

  recoil_from_tracking_ = parameters.get<bool>("recoil_from_tracking");
  track_collection_ = parameters.get<std::string>("track_collection");
  track_pass_name_ = parameters.get<std::string>("track_pass_name");

  sp_collection_ = parameters.get<std::string>("sp_coll_name");
  sp_pass_name_ = parameters.get<std::string>("sp_pass_name");
  sim_particles_pass_name_ =
      parameters.get<std::string>("sim_particles_pass_name");
}

bool VisiblesVetoProcessor::inList(std::vector<int> parents, int track_id) {
  return std::find(parents.begin(), parents.end(), track_id) != parents.end();
}

void VisiblesVetoProcessor::clearProcessor() {
  bdt_features_.clear();

  n_layers_hit_ = 0;
  x_std_ = 0.;
  y_std_ = 0.;
  z_std_ = 0.;
  x_mean_ = 0.;
  y_mean_ = 0.;
  r_mean_ = 0.;
  iso_hits_ = 0;
  iso_energy_ = 0.;
  n_readout_hits_ = 0;
  summed_det_ = 0.;
  r_mean_from_photon_track_ = 0.;
}

void VisiblesVetoProcessor::produce(framework::Event &event) {
  ldmx::VisiblesVetoResult result;

  clearProcessor();

  const auto &particle_map{event.getMap<int, ldmx::SimParticle>(
      "SimParticles", sim_particles_pass_name_)};

  // Get target scoring plane hits for recoil electron
  // Use this to calculate the projected photon line vector
  // This currently uses truth-level information, but it should be replaced
  // by reconstructed tracker information, when available
  std::vector<double> gamma_p(3);
  std::vector<double> gamma_x0(3);
  if (recoil_from_tracking_) {
    const auto &recoil_tracks{
        event.getCollection<ldmx::NewTrack>(track_collection_, track_pass_name_)};
    // Fill this in later when you know how to use it
    for (auto &track : recoil_tracks) {
      // need to figure out how to best isolate candidate electron track
      auto trk_pos = track.getPositionAtTarget();
      auto trk_mom = track.getMomentumAtTarget();
      if (track.getCharge() == 1 && track.getNhits() == 5 &&
          trk_pos.size() == 3 && trk_mom.size() == 3) {
        gamma_x0 = trk_pos;
        gamma_p[0] = -1. * trk_mom[0];
        gamma_p[1] = -1. * trk_mom[1];
        gamma_p[2] = beam_energy_mev_ - trk_mom[2];
      }
    }
  } else {
    if (event.exists(sp_collection_, sp_pass_name_)) {
      const auto &target_sp_hits = event.getCollection<ldmx::SimTrackerHit>(
          sp_collection_, sp_pass_name_);
      for (auto const &it : particle_map) {
        for (auto const &sphit : target_sp_hits) {
          if (sphit.getPosition()[2] > 0) {
            if (it.first == sphit.getTrackID()) {
              if (it.second.getPdgID() == 11 &&
                  inList(it.second.getParents(), 0)) {
                /* Since SP hit positions are stored as floats and gamma_x0 is
                 a double vector, the conversion here is a little convolcuted.
               */
                std::vector<float> x0_float = sphit.getPosition();
                std::vector<double> x0_double(x0_float.begin(), x0_float.end());
                gamma_x0 = x0_double;
                gamma_p[0] = -1. * sphit.getMomentum()[0];
                gamma_p[1] = -1. * sphit.getMomentum()[1];
                gamma_p[2] = beam_energy_mev_ - sphit.getMomentum()[2];
              }
            }
          }
        }
      }
    }
  }

  // Get Hcal reconstructed hits and loop through them to build features
  const auto &hcal_rec_hits =
      event.getCollection<ldmx::HcalHit>(rec_coll_name_, rec_pass_name_);

  double z_mean = 0.;  // need this when calculating z_std_
  std::vector<int> layers_hit;
  for (const ldmx::HcalHit &hit : hcal_rec_hits) {
    if (hit.getEnergy() > 0.) {
      ldmx::HcalID det_id(hit.getID());
      if (det_id.getSection() != 0) {  // skip hits that aren't in main Hcal
        continue;
      }
      n_readout_hits_ += 1;
      double hit_x = hit.getXPos();
      double hit_y = hit.getYPos();
      double hit_z = hit.getZPos();
      double hit_r = sqrt(hit_x * hit_x + hit_y * hit_y);

      summed_det_ += hit.getEnergy();

      x_mean_ += hit_x * hit.getEnergy();
      y_mean_ += hit_y * hit.getEnergy();
      z_mean += hit_z * hit.getEnergy();
      r_mean_ += hit_r * hit.getEnergy();

      // check if this is a new layer in the collection
      if (!(std::find(layers_hit.begin(), layers_hit.end(),
                      det_id.getLayerID()) != layers_hit.end())) {
        layers_hit.push_back(det_id.getLayerID());
      }

      double proj_x =
          gamma_x0[0] + (hit_z - gamma_x0[2]) * gamma_p[0] / gamma_p[2];
      double proj_y =
          gamma_x0[1] + (hit_z - gamma_x0[2]) * gamma_p[1] / gamma_p[2];

      r_mean_from_photon_track_ +=
          hit.getEnergy() * sqrt((hit_x - proj_x) * (hit_x - proj_x) +
                                 (hit_y - proj_y) * (hit_y - proj_y));

      // Calculate isolated hits
      double closest_point = 9999.;
      for (const ldmx::HcalHit &hit2 : hcal_rec_hits) {
        if (hit2.getEnergy() > 0.) {
          ldmx::HcalID det_i_d2(hit2.getID());
          if (det_i_d2.getLayerID() == det_id.getLayerID()) {
            // Determine if a bar is vertical (along y-axis) or horizontal
            // (along x-axis) Odd layers have horizontal strips Even layers have
            // vertical strips
            if (hit.isOrientationX()) {
              if (abs(hit2.getYPos() - hit_y) > 0) {
                if (abs(hit2.getYPos() - hit_y) < closest_point) {
                  closest_point = abs(hit2.getYPos() - hit_y);
                }
              }
            }
            if (hit.isOrientationY()) {
              if (abs(hit2.getXPos() - hit_x) > 0) {
                if (abs(hit2.getXPos() - hit_x) < closest_point) {
                  closest_point = abs(hit2.getXPos() - hit_x);
                }
              }
            }
          }
        }
      }
      if (closest_point > 50.) {
        iso_hits_ += 1;
        iso_energy_ += hit.getEnergy();
      }
    }
  }

  n_layers_hit_ = layers_hit.size();

  if (summed_det_ > 0.) {
    x_mean_ /= summed_det_;
    y_mean_ /= summed_det_;
    z_mean /= summed_det_;
    r_mean_ /= summed_det_;

    r_mean_from_photon_track_ /= summed_det_;
  }

  for (const ldmx::HcalHit &hit : hcal_rec_hits) {
    if (hit.getEnergy() > 0.) {
      ldmx::HcalID det_id(hit.getID());
      if (det_id.getSection() == 0) {
        x_std_ += hit.getEnergy() * (hit.getXPos() - x_mean_) *
                  (hit.getXPos() - x_mean_);
        y_std_ += hit.getEnergy() * (hit.getYPos() - y_mean_) *
                  (hit.getYPos() - y_mean_);
        z_std_ += hit.getEnergy() * (hit.getZPos() - z_mean) *
                  (hit.getZPos() - z_mean);
      }
    }
  }

  if (summed_det_ > 0.) {
    x_std_ = sqrt(x_std_ / summed_det_);
    y_std_ = sqrt(y_std_ / summed_det_);
    z_std_ = sqrt(z_std_ / summed_det_);
  }

  result.setVariables(n_layers_hit_, x_std_, y_std_, z_std_, x_mean_, y_mean_,
                      r_mean_, iso_hits_, iso_energy_, n_readout_hits_,
                      summed_det_, r_mean_from_photon_track_);

  buildBDTFeatureVector(result);

  ldmx::ort::FloatArrays inputs({bdt_features_});
  float pred =
      rt_->run({feature_list_name_}, inputs, {"probabilities"})[0].at(1);
  ldmx_log(info) << " Visibles BDT was ran, score is " << pred;

  event.add(collection_name_, result);
}

}  // namespace hcal

DECLARE_PRODUCER(hcal::VisiblesVetoProcessor);
