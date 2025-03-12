/**
 * @file EcalWABRecProcessor.h
 * @brief Processor that reconstructs important kinematic variables for WAB
 * studies
 * @author Sanjit Masanam, UCSB
 */

#include "Ecal/EcalWABRecProcessor.h"

// LDMX
#include "DetDescr/SimSpecialID.h"
#include "Ecal/Event/EcalHit.h"
#include "Recon/Event/EventConstants.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Tracking/Event/StraightTrack.h"

/*~~~~~~~~~~~*/
/*   Tools   */
/*~~~~~~~~~~~*/
#include "Eigen/Dense"
#include "Tools/AnalysisUtils.h"

// C++
#include <stdlib.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <numbers>  // For std::numbers::pi
#include <numeric>
#include <tuple>
#include <vector>

// ROOT (MIP tracking)
#include "TDecompSVD.h"
#include "TMatrixD.h"
#include "TVector3.h"

namespace ecal {

// 68% Electron Radii of Containment for various theta ranges
const std::vector<double> radius_68_theta_0_to_10 = {
    10.12233413, 9.921772,    11.38255086, 11.67991867, 13.14337347,
    13.17120624, 16.80994665, 17.83787244, 22.44684374, 23.74239886,
    28.60564083, 30.27889678, 34.86404888, 36.39009394, 41.29309474,
    43.34682279, 48.55982854, 50.80565589, 55.29496257, 57.92737879,
    60.64828824, 65.51760517, 68.26709803, 76.32877518, 84.61219467,
    98.21320491, 110.9880892, 120.6762931, 140.6174478, 136.4979268,
    145.579465,  154.9803228, 164.7005,    174.7399968};

const std::vector<double> radius_68_theta_10_to_15 = {
    10.82307758, 11.17850518, 16.2185281,  18.62488713, 22.63408229,
    24.71769042, 30.11217538, 32.69939046, 37.99753196, 40.81619543,
    45.89054775, 49.03066318, 54.00440948, 59.31733555, 63.40789682,
    64.77580021, 73.00113678, 73.25561396, 78.8914776,  86.73962133,
    97.05926327, 96.6932739,  111.6226151, 106.5960265, 109.477541,
    128.0220711, 145.4137195, 210.3582819, 199.6355662, 184.2513208,
    195.076552,  206.2322029, 217.7182737, 229.5347642};

const std::vector<double> radius_68_theta_15_to_20 = {
    12.79450901, 13.02698578, 21.27450933, 25.66008312, 31.78592103,
    35.99689874, 44.37101115, 48.82709363, 55.05972458, 59.68948687,
    65.39866214, 70.59280337, 76.06007787, 82.22695257, 87.50371819,
    90.60099831, 96.34848268, 101.4928478, 106.7157092, 105.0540604,
    110.0653355, 148.3428736, 133.1449443, 146.997265,  173.3954389,
    175.4329544, 184.3003543, 259.8415751, 215.0165,    231.1288534,
    243.4440277, 256.085458,  269.0531444, 282.3470868};

const std::vector<double> radius_68_theta_20_to_30 = {
    14.16989595, 15.4488322,  28.31044668, 37.54285657, 48.57288885,
    57.04243339, 68.99836079, 75.33388728, 85.00572867, 91.52574074,
    102.5044698, 106.5315986, 116.2341378, 127.1121442, 133.8866375,
    144.5121759, 162.1726963, 160.2986579, 171.386638,  182.5653112,
    205.5853241, 196.3113071, 200.5907513, 228.7275694, 234.0298491,
    251.7701385, 293.9351568, 310.521898,  344.1455457, 293.3518953,
    303.2401036, 313.128312,  323.0165203, 332.9047287};

const std::vector<double> radius_68_theta_30_to_90 = {
    22.50983127, 26.44537503, 58.24642887, 90.59076279, 130.0592014,
    157.4611392, 184.2187293, 202.6994588, 225.3488816, 243.3454167,
    269.2456428, 280.6119298, 303.8591523, 322.0522722, 335.1780181,
    350.3398234, 353.7763544, 373.9942362, 382.9453608, 401.9703438,
    441.6281859, 432.5241826, 455.2878243, 492.2888656, 502.6653722,
    480.2334627, 566.5438302, 505.7032783, 556.1650321, 596.9112032,
    616.1614593, 635.4117154, 654.6619715, 673.9122276};

void EcalWABRecProcessor::onProcessStart() {}

void EcalWABRecProcessor::configure(framework::config::Parameters& parameters) {
  // Set the collection name as defined in the configuration
  collection_name_ = parameters.getParameter<std::string>("collection_name");
  rec_pass_name_ = parameters.getParameter<std::string>("rec_pass_name");
  rec_coll_name_ = parameters.getParameter<std::string>("rec_coll_name");
  track_pass_name_ = parameters.getParameter<std::string>("track_pass_name");
  track_coll_name_ = parameters.getParameter<std::string>("track_coll_name");
}

void EcalWABRecProcessor::produce(framework::Event& event) {
  // Define start time for processing
  auto start = std::chrono::high_resolution_clock::now();
  nevents_++;

  // Keep track of event progress where:
  // 0: No tracks found
  // 1: Track found but not enough info to reconstruct either electron or photon
  // 2: Track found and enough info to reconstruct electron
  // 3: Track found and enough info to reconstruct electron and photon
  int progress_num = 0;

  // Get the Ecal Geometry
  geometry_ = &getCondition<ldmx::EcalGeometry>(
      ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  // Get the collection of ecal_rec_hits and tracks
  const std::vector<ldmx::EcalHit> ecal_rec_hits =
      event.getCollection<ldmx::EcalHit>(rec_coll_name_, rec_pass_name_);
  const std::vector<ldmx::StraightTrack> linear_tracks =
      event.getCollection<ldmx::StraightTrack>(track_coll_name_,
                                               track_pass_name_);

  // Define variables to save recoil electron/photon information (SP)
  std::vector<double> recoil_e_p, recoil_y_p;
  std::vector<float> recoil_e_pos, recoil_y_pos;

  // Result object that stores kinematic variables
  ldmx::EcalWABResult result;

  // Define kinematic variables
  const std::vector<double> z_hat = {0, 0, 1};
  double true_theta_electron = -9.;
  double true_theta_photon = -9.;
  double true_phi_electron = -9.;
  double true_phi_photon = -9.;
  double rec_theta_electron = -9.;
  double rec_theta_photon = -9.;
  double rec_phi_electron = -9.;
  double rec_phi_photon = -9.;
  double true_theta_diff_electron_photon = -9.;
  double true_phi_diff_electron_photon = -9.;
  double rec_theta_diff_electron_photon = -9.;
  double rec_phi_diff_electron_photon = -9.;
  double true_rec_theta_diff_electron = -9.;
  double true_rec_phi_diff_electron = -9.;
  double true_rec_theta_diff_photon = -9.;
  double true_rec_phi_diff_photon = -9.;
  float true_electron_shower_energy = -999.;
  float true_photon_shower_energy = -999.;
  double rec_electron_shower_energy = -999.;
  double rec_photon_shower_energy = -999.;

  // Create lists for rec hits and electron/photon shower hits
  std::vector<std::array<double, 6>> rec_hit_list;
  std::vector<std::array<double, 6>> ele_hit_list;
  std::vector<std::array<double, 6>> phot_hit_list;

  // Save rec hit info to rec_hit_list
  for (const ldmx::EcalHit& hit : ecal_rec_hits) {
    ldmx::EcalID id(hit.getID());
    auto [x, y, z] = geometry_->getPosition(id);
    double energy = hit.getEnergy();
    double layer_num = id.layer();
    rec_hit_list.push_back({x, y, z, layer_num, 0, energy});
  }

  if (event.exists("TargetScoringPlaneHits")) {
    //
    // Loop through all of the sim particles and find the recoil
    // photon/electron.
    //

    // Find Target SP hit for recoil photon/electron
    const std::vector<ldmx::SimTrackerHit> target_sp_hits =
        event.getCollection<ldmx::SimTrackerHit>("TargetScoringPlaneHits");
    float photon_p_zmax = 0, electron_p_zmax = 0;
    for (const ldmx::SimTrackerHit& sp_hit : target_sp_hits) {
      ldmx::SimSpecialID hit_id(sp_hit.getID());
      if (hit_id.plane() != 1 || sp_hit.getMomentum()[2] <= 0) continue;

      if (sp_hit.getPdgID() == 11) {
        if (sp_hit.getMomentum()[2] > electron_p_zmax) {
          recoil_e_p = sp_hit.getMomentum();
          true_electron_shower_energy = sp_hit.getEnergy();
          recoil_e_pos = sp_hit.getPosition();
          electron_p_zmax = recoil_e_p[2];
        }
      }
      if (sp_hit.getPdgID() == 22) {
        if (sp_hit.getMomentum()[2] > photon_p_zmax) {
          recoil_y_p = sp_hit.getMomentum();
          true_photon_shower_energy = sp_hit.getEnergy();
          recoil_y_pos = sp_hit.getPosition();
          photon_p_zmax = recoil_y_p[2];
        }
      }
    }

    // Calculating true theta values using SP hit parameters
    if (recoil_y_p.size() == 3 &&
        std::sqrt(std::pow(recoil_y_p[0], 2) + std::pow(recoil_y_p[1], 2) +
                  std::pow(recoil_y_p[2], 2)) != 0 &&
        recoil_e_p.size() == 3 &&
        std::sqrt(std::pow(recoil_e_p[0], 2) + std::pow(recoil_e_p[1], 2) +
                  std::pow(recoil_e_p[2], 2)) != 0) {
      true_theta_electron =
          (180 / std::numbers::pi) *
          std::acos(std::inner_product(recoil_e_p.begin(), recoil_e_p.end(),
                                       z_hat.begin(), 0.0) /
                    std::sqrt(std::pow(recoil_e_p[0], 2) +
                              std::pow(recoil_e_p[1], 2) +
                              std::pow(recoil_e_p[2], 2)));
      true_theta_photon =
          (180 / std::numbers::pi) *
          std::acos(std::inner_product(recoil_y_p.begin(), recoil_y_p.end(),
                                       z_hat.begin(), 0.0) /
                    std::sqrt(std::pow(recoil_y_p[0], 2) +
                              std::pow(recoil_y_p[1], 2) +
                              std::pow(recoil_y_p[2], 2)));
    }

    // Calculating true phi values using SP hit parameters
    if (recoil_y_p.size() == 3 && recoil_y_p[2] != 0 &&
        recoil_e_p.size() == 3 && recoil_e_p[2] != 0) {
      true_phi_electron =
          (180 / std::numbers::pi) * std::atan(recoil_e_p[1] / recoil_e_p[0]);
      if (recoil_e_p[1] < 0) {
        true_phi_electron += 180;
      }
      if (recoil_e_p[0] < 0 && recoil_e_p[1] > 0) {
        true_phi_electron += 360;
      }
      true_phi_photon =
          (180 / std::numbers::pi) * std::atan(recoil_y_p[1] / recoil_y_p[0]);
      if (recoil_y_p[1] < 0) {
        true_phi_photon += 180;
      }
      if (recoil_y_p[0] < 0 && recoil_y_p[1] > 0) {
        true_phi_photon += 360;
      }
    }

    // Calculating true delta_phi/delta_theta using SP hit parameters
    if (recoil_y_p.size() == 3 && recoil_e_p.size() == 3) {
      std::array<double, 2> phi_diff_electron_arr = {recoil_e_p[0],
                                                     recoil_e_p[1]};
      std::array<double, 2> phi_diff_photon_arr = {recoil_y_p[0],
                                                   recoil_y_p[1]};
      std::array<double, 2> theta_diff_electron_arr = {recoil_e_p[2],
                                                       recoil_e_p[0]};
      std::array<double, 2> theta_diff_photon_arr = {recoil_y_p[2],
                                                     recoil_y_p[0]};

      true_theta_diff_electron_photon =
          (180 / std::numbers::pi) *
          std::acos(std::inner_product(theta_diff_electron_arr.begin(),
                                       theta_diff_electron_arr.end(),
                                       theta_diff_photon_arr.begin(), 0.0) /
                    (std::sqrt(std::pow(theta_diff_electron_arr[0], 2) +
                               std::pow(theta_diff_electron_arr[1], 2)) *
                     std::sqrt(std::pow(theta_diff_photon_arr[0], 2) +
                               std::pow(theta_diff_photon_arr[1], 2))));
      true_phi_diff_electron_photon =
          (180 / std::numbers::pi) *
          std::acos(std::inner_product(phi_diff_electron_arr.begin(),
                                       phi_diff_electron_arr.end(),
                                       phi_diff_photon_arr.begin(), 0.0) /
                    (std::sqrt(std::pow(phi_diff_electron_arr[0], 2) +
                               std::pow(phi_diff_electron_arr[1], 2)) *
                     std::sqrt(std::pow(phi_diff_photon_arr[0], 2) +
                               std::pow(phi_diff_photon_arr[1], 2))));
    }
  }

  // Defining variables to save best fit results
  std::pair<Eigen::VectorXd, Eigen::VectorXd> linear_fit_coeffs;
  std::tuple<Eigen::VectorXd, double, int, Eigen::MatrixXd, int> best_x_result;
  std::tuple<Eigen::VectorXd, double, int, Eigen::MatrixXd, int> best_y_result;
  std::get<1>(best_x_result) = 10e99;
  std::get<1>(best_y_result) = 10e99;

  // Looping over tracks to find best fit to hits
  std::vector<double> ele_roc = radius_68_theta_30_to_90;
  for (const ldmx::StraightTrack& track : linear_tracks) {
    progress_num = 1;
    // Determining the RoC value to use based on recoil electron (track) theta
    std::vector<double> track_vec = {track.getSlopeX(), track.getSlopeY(), 1};
    double track_theta =
        (180 / std::numbers::pi) *
        std::acos(std::inner_product(track_vec.begin(), track_vec.end(),
                                     z_hat.begin(), 0.0) /
                  std::sqrt(std::pow(track_vec[0], 2) +
                            std::pow(track_vec[1], 2) + 1));
    if (track_theta <= 10) {
      ele_roc = radius_68_theta_0_to_10;
    } else if (track_theta > 10 && track_theta <= 15) {
      ele_roc = radius_68_theta_10_to_15;
    } else if (track_theta > 15 && track_theta <= 20) {
      ele_roc = radius_68_theta_15_to_20;
    } else if (track_theta > 20 && track_theta <= 30) {
      ele_roc = radius_68_theta_20_to_30;
    }

    // Labeling hits as electron (1) or photon (0)
    for (std::array<double, 6>& hit : rec_hit_list) {
      if (std::sqrt(std::pow(hit[0] - (track.getSlopeX() * hit[2] +
                                       track.getInterceptX()),
                             2) +
                    std::pow(hit[1] - (track.getSlopeY() * hit[2] +
                                       track.getInterceptY()),
                             2)) < ele_roc[hit[3]]) {
        hit[4] = 1;
      }
    }
    // Create vectors to hold electron/photon hits specifically
    std::vector<double> ele_hit_list_x, ele_hit_list_y, ele_hit_list_z;
    std::vector<double> phot_hit_list_x, phot_hit_list_y, phot_hit_list_z;

    // Use labels to sort hits as electron/photon and calculate shower energies
    for (const auto& hit : rec_hit_list) {
      if (hit[4] == 1) {
        ele_hit_list.push_back(hit);
        ele_hit_list_x.push_back(hit[0]);
        ele_hit_list_y.push_back(hit[1]);
        ele_hit_list_z.push_back(hit[2]);
      } else if (hit[4] == 0) {
        phot_hit_list.push_back(hit);
        phot_hit_list_x.push_back(hit[0]);
        phot_hit_list_y.push_back(hit[1]);
        phot_hit_list_z.push_back(hit[2]);
      }
    }

    // Fit both photon/electron or just electron hits based on # of viable
    // showers
    if (phot_hit_list.size() >= 3 && ele_hit_list.size() >= 3) {
      progress_num =
          3;  // Set progress_num to 3 to halt electron-only reconstruction

      // Generate guesses and error vectors for vertex constrained fit
      std::vector<double> x_guess = {
          track.getSlopeX(),
          (phot_hit_list.back()[0] - phot_hit_list[0][0]) /
              (phot_hit_list.back()[2] - phot_hit_list[0][2]),
          track.getInterceptX()};
      std::vector<double> y_guess = {
          track.getSlopeY(),
          (phot_hit_list.back()[1] - phot_hit_list[0][1]) /
              (phot_hit_list.back()[2] - phot_hit_list[0][2]),
          track.getInterceptY()};
      std::vector<double> phot_hit_error(phot_hit_list.size(),
                                         0.456435464588 * 4.816);
      std::vector<double> ele_hit_error(ele_hit_list.size(),
                                        0.456435464588 * 4.816);

      int max_iter = 200;
      // Carry out fit
      std::tuple<Eigen::VectorXd, double, int, Eigen::MatrixXd, int> x_result =
          fit2DTracksConstrained(ele_hit_list_z, ele_hit_list_x, ele_hit_error,
                                 phot_hit_list_z, phot_hit_list_x,
                                 phot_hit_error, x_guess, max_iter, 0, 0.001,
                                 10.0);

      std::tuple<Eigen::VectorXd, double, int, Eigen::MatrixXd, int> y_result =
          fit2DTracksConstrained(ele_hit_list_z, ele_hit_list_y, ele_hit_error,
                                 phot_hit_list_z, phot_hit_list_y,
                                 phot_hit_error, y_guess, max_iter, 0, 0.001,
                                 40.0);

      // Update best fit variables if current fit is an improvement
      if ((std::get<1>(x_result) + std::get<1>(y_result)) / 2 <
          (std::get<1>(best_x_result) + std::get<1>(best_y_result)) / 2) {
        best_x_result = x_result;
        best_y_result = y_result;
      }
    }

    // If there isn't enough info for both electron and photon reconstruction,
    // reconstruct electron if possible
    else if (ele_hit_list.size() >= 3) {
      if (progress_num != 3) {
        progress_num = 2;  // Set progress_num to 3 to indicate electron-only
                           // reconstruction
        linear_fit_coeffs =
            polyfitXYvsZ(ele_hit_list_x, ele_hit_list_y, ele_hit_list_z, 1);
      }
    }
  }

  // Calculate kinematic variables for electron and/or photon
  // based on # of viable showers (with reconstruction information)
  if (std::get<0>(best_x_result).size() != 0) {
    rec_electron_shower_energy = 0;
    rec_photon_shower_energy = 0;
    for (const auto& hit : ele_hit_list) {
      rec_electron_shower_energy += hit[5];
    }
    for (const auto& hit : phot_hit_list) {
      rec_photon_shower_energy += hit[5];
    }

    std::vector<double> ele_params = {std::get<0>(best_x_result)(0),
                                      std::get<0>(best_y_result)(0)};
    std::vector<double> phot_params = {std::get<0>(best_x_result)(1),
                                       std::get<0>(best_y_result)(1)};
    std::vector<double> ele_params_x = {std::get<0>(best_x_result)(0)};
    std::vector<double> phot_params_x = {std::get<0>(best_x_result)(1)};

    rec_theta_electron =
        (180 / std::numbers::pi) *
        std::acos(1 / std::sqrt(std::pow(ele_params[0], 2) +
                                std::pow(ele_params[1], 2) + 1));
    rec_theta_photon =
        (180 / std::numbers::pi) *
        std::acos(1 / std::sqrt(std::pow(phot_params[0], 2) +
                                std::pow(phot_params[1], 2) + 1));

    rec_phi_electron =
        (180 / std::numbers::pi) * std::atan(ele_params[1] / ele_params[0]);
    if (ele_params[1] < 0) {
      rec_phi_electron += 180;
    }
    if (ele_params[0] < 0 && ele_params[1] > 0) {
      rec_phi_electron += 360;
    }
    rec_phi_photon =
        (180 / std::numbers::pi) * std::atan(phot_params[1] / phot_params[0]);
    if (phot_params[1] < 0) {
      rec_phi_photon += 180;
    }
    if (phot_params[0] < 0 && phot_params[1] > 0) {
      rec_phi_photon += 360;
    }

    rec_theta_diff_electron_photon =
        (180 / std::numbers::pi) *
        std::acos(std::inner_product(ele_params_x.begin(), ele_params_x.end(),
                                     phot_params_x.begin(), 1.0) /
                  (std::sqrt(std::pow(ele_params_x[0], 2) + std::pow(1, 2)) *
                   std::sqrt(std::pow(phot_params_x[0], 2) + std::pow(1, 2))));
    rec_phi_diff_electron_photon =
        (180 / std::numbers::pi) *
        std::acos(std::inner_product(ele_params.begin(), ele_params.end(),
                                     phot_params.begin(), 0.0) /
                  (std::sqrt(std::pow(ele_params[0], 2) +
                             std::pow(ele_params[1], 2)) *
                   std::sqrt(std::pow(phot_params[0], 2) +
                             std::pow(phot_params[1], 2))));

    if (recoil_y_p.size() == 3 && recoil_y_p[2] != 0 &&
        recoil_e_p.size() == 3 && recoil_e_p[2] != 0) {
      true_rec_theta_diff_electron =
          (180 / std::numbers::pi) *
          std::acos(std::inner_product(ele_params_x.begin(), ele_params_x.end(),
                                       recoil_e_p.begin(), recoil_e_p[2]) /
                    (std::sqrt(std::pow(ele_params_x[0], 2) + std::pow(1, 2)) *
                     std::sqrt(std::pow(recoil_e_p[0], 2) +
                               std::pow(recoil_e_p[2], 2))));
      true_rec_theta_diff_photon =
          (180 / std::numbers::pi) *
          std::acos(std::inner_product(phot_params_x.begin(),
                                       phot_params_x.end(), recoil_y_p.begin(),
                                       recoil_y_p[2]) /
                    (std::sqrt(std::pow(phot_params_x[0], 2) + std::pow(1, 2)) *
                     std::sqrt(std::pow(recoil_y_p[0], 2) +
                               std::pow(recoil_y_p[2], 2))));
      true_rec_phi_diff_electron =
          (180 / std::numbers::pi) *
          std::acos(std::inner_product(ele_params.begin(), ele_params.end(),
                                       recoil_e_p.begin(), 0) /
                    (std::sqrt(std::pow(ele_params[0], 2) +
                               std::pow(ele_params[1], 2)) *
                     std::sqrt(std::pow(recoil_e_p[0], 2) +
                               std::pow(recoil_e_p[1], 2))));
      true_rec_phi_diff_photon =
          (180 / std::numbers::pi) *
          std::acos(std::inner_product(phot_params.begin(), phot_params.end(),
                                       recoil_y_p.begin(), 0) /
                    (std::sqrt(std::pow(phot_params[0], 2) +
                               std::pow(phot_params[1], 2)) *
                     std::sqrt(std::pow(recoil_y_p[0], 2) +
                               std::pow(recoil_y_p[1], 2))));
    }
  } else if (progress_num == 2) {
    rec_electron_shower_energy = 0;
    for (const auto& hit : ele_hit_list) {
      rec_electron_shower_energy += hit[5];
    }

    std::vector<double> ele_params = {linear_fit_coeffs.first(1),
                                      linear_fit_coeffs.second(1)};
    std::vector<double> ele_params_x = {linear_fit_coeffs.first(1)};

    rec_theta_electron =
        (180 / std::numbers::pi) *
        std::acos(1 / std::sqrt(std::pow(ele_params[0], 2) +
                                std::pow(ele_params[1], 2) + 1));
    rec_phi_electron =
        (180 / std::numbers::pi) * std::atan(ele_params[1] / ele_params[0]);
    if (ele_params[1] < 0) {
      rec_phi_electron += 180;
    }
    if (ele_params[0] < 0 && ele_params[1] > 0) {
      rec_phi_electron += 360;
    }

    if (recoil_y_p.size() == 3 && recoil_y_p[2] != 0 &&
        recoil_e_p.size() == 3 && recoil_e_p[2] != 0) {
      true_rec_theta_diff_electron =
          (180 / std::numbers::pi) *
          std::acos(std::inner_product(ele_params_x.begin(), ele_params_x.end(),
                                       recoil_e_p.begin(), recoil_e_p[2]) /
                    (std::sqrt(std::pow(ele_params_x[0], 2) + std::pow(1, 2)) *
                     std::sqrt(std::pow(recoil_e_p[0], 2) +
                               std::pow(recoil_e_p[2], 2))));
      true_rec_phi_diff_electron =
          (180 / std::numbers::pi) *
          std::acos(std::inner_product(ele_params.begin(), ele_params.end(),
                                       recoil_e_p.begin(), 0) /
                    (std::sqrt(std::pow(ele_params[0], 2) +
                               std::pow(ele_params[1], 2)) *
                     std::sqrt(std::pow(recoil_e_p[0], 2) +
                               std::pow(recoil_e_p[1], 2))));
    }

    // Set photon variables to non-physical value
    // that corresponds to electron-only reco case
    rec_theta_photon = -5.;
    rec_phi_photon = -5.;
    rec_theta_diff_electron_photon = -5.;
    rec_phi_diff_electron_photon = -5.;
    true_rec_theta_diff_photon = -5.;
    true_rec_phi_diff_photon = -5.;
  }

  // Setting output object equal to calculated variables
  result.setVariables(
      true_theta_electron, true_theta_photon, true_phi_electron,
      true_phi_photon, rec_theta_electron, rec_theta_photon, rec_phi_electron,
      rec_phi_photon, true_theta_diff_electron_photon,
      true_phi_diff_electron_photon, rec_theta_diff_electron_photon,
      rec_phi_diff_electron_photon, true_rec_theta_diff_electron,
      true_rec_phi_diff_electron, true_rec_theta_diff_photon,
      true_rec_phi_diff_photon, true_electron_shower_energy,
      true_photon_shower_energy, rec_electron_shower_energy,
      rec_photon_shower_energy, progress_num);

  event.add(collection_name_, result);

  // Calculate processing time for event
  auto end = std::chrono::high_resolution_clock::now();
  auto diff = end - start;
  processing_time_ += std::chrono::duration<double, std::milli>(diff).count();
}

void EcalWABRecProcessor::onProcessEnd() {
  ldmx_log(info) << "AVG Time/Event: " << std::fixed << std::setprecision(2)
                 << processing_time_ / nevents_ << " ms";
}

std::tuple<Eigen::VectorXd, double, int, Eigen::MatrixXd, int>
EcalWABRecProcessor::fit2DTracksConstrained(
    const std::vector<double>& x1, const std::vector<double>& y1,
    const std::vector<double>& s1, const std::vector<double>& x2,
    const std::vector<double>& y2, const std::vector<double>& s2,
    const std::vector<double>& guess, int max_iter, int verbose, double d_chisq,
    double abs_lim) {
  /*
    Function that fits two 2D tracks with a vertex constraint (same intercepts).
    The fitted model is initially defined as:
      y1 = par[0] * x1 + abs_lim * tanh(par[2] / abs_lim)
      y2 = par[1] * x2 + abs_lim * tanh(par[2] / abs_lim)
    After updating parameters the fitted values are computed as:
      y1 = par[0] * (x1 - par[2])
      y2 = par[1] * (x2 - par[2])

    Inputs:
      x1, y1, s1 : measured coordinates and errors for track 1
      x2, y2, s2 : measured coordinates and errors for track 2
      guess     : initial guess for the parameter vector (size 3)
      max_iter   : maximum number of iterations (default 20)
      verbose : level of verbose (default 0)
      d_chisq    : stopping criterion for chi-squared improvement (default
    0.001) abs_lim   : a parameter used in the fit (default 10) Returns: A tuple
    containing: par    : fitted parameters (Eigen::VectorXd of size 3) chisq  :
    chi-squared at minimum (double) ndof   : number of degrees of freedom (int)
        cov    : covariance matrix (Eigen::MatrixXd 3x3)
        niter  : number of iterations used (int)
  */
  // Copy the initial guess into a 3-element parameter vector
  Eigen::VectorXd par(3);
  par(0) = guess[0];
  par(1) = guess[1];
  par(2) = guess[2];

  // Determine number of points in each track and total
  int n1 = x1.size();
  int n2 = x2.size();
  int n = n1 + n2;

  // Concatenate x, y, and s into Eigen vectors of size n.
  Eigen::VectorXd x(n), y(n), s(n);
  for (int i = 0; i < n1; ++i) {
    x(i) = x1[i];
    y(i) = y1[i];
    s(i) = s1[i];
  }
  for (int i = 0; i < n2; ++i) {
    x(n1 + i) = x2[i];
    y(n1 + i) = y2[i];
    s(n1 + i) = s2[i];
  }

  // Build the weight matrix W = diag(1/s_i^2)
  Eigen::MatrixXd W = Eigen::MatrixXd::Zero(n, n);
  for (int i = 0; i < n; ++i) {
    W(i, i) = 1.0 / (s(i) * s(i));
  }

  double chi_sq = 0.0;
  double old_chi_sq = 1e12;  // a large initial value
  int n_iter = 0;
  Eigen::MatrixXd cov(3, 3);  // covariance matrix

  // Iterative fitting loop
  for (int iter = 0; iter < max_iter; ++iter) {
    n_iter = iter + 1;

    // Compute fitted y coordinates for each track using the current parameters.
    // For track 1: y1_fit = par[0] * x1 + abs_lim * tanh(par[2]/abs_lim)
    // For track 2: y2_fit = par[1] * x2 + abs_lim * tanh(par[2]/abs_lim)
    Eigen::VectorXd y1_fit(n1), y2_fit(n2);
    double tanh_term = std::tanh(par(2) / abs_lim);
    for (int i = 0; i < n1; ++i) {
      y1_fit(i) = par(0) * x1[i] + abs_lim * tanh_term;
    }
    for (int i = 0; i < n2; ++i) {
      y2_fit(i) = par(1) * x2[i] + abs_lim * tanh_term;
    }

    // Concatenate the fitted values
    Eigen::VectorXd y_fit(n);
    for (int i = 0; i < n1; ++i) {
      y_fit(i) = y1_fit(i);
    }
    for (int i = 0; i < n2; ++i) {
      y_fit(n1 + i) = y2_fit(i);
    }

    // Compute chi-squared: sum_i [ (y_fit[i]-y[i])^2 / s[i]^2 ]
    chi_sq = 0.0;
    for (int i = 0; i < n; ++i) {
      double diff = y_fit(i) - y(i);
      chi_sq += (diff * diff) / (s(i) * s(i));
    }

    if (verbose > 0) {
      ldmx_log(debug) << "Before iteration " << iter << ", chi_sq = " << chi_sq;
      ldmx_log(debug) << "Track 1 residuals: ";
      for (int i = 0; i < n1; ++i) {
        ldmx_log(debug) << (y1_fit(i) - y1[i]);
      }
      ldmx_log(debug) << "Track 2 residuals: ";
      for (int i = 0; i < n2; ++i) {
        ldmx_log(debug) << (y2_fit(i) - y2[i]);
      }
    }

    // Compute the derivatives (Jacobian components)
    // For track 1:
    //   dy1/dpar0 = x1,   dy1/dpar1 = 0,   dy1/dpar2 =
    //   (1/cosh(par[2]/abs_lim))^2 (constant for all points)
    // For track 2:
    //   dy2/dpar0 = 0,   dy2/dpar1 = x2,   dy2/dpar2 =
    //   (1/cosh(par[2]/abs_lim))^2
    Eigen::VectorXd dy1_dpar_0(n1), dy1_dpar_1 = Eigen::VectorXd::Zero(n1),
                                    dy1_dpar_2(n1);
    Eigen::VectorXd dy2_dpar_0 = Eigen::VectorXd::Zero(n2), dy2_dpar_1(n2),
                    dy2_dpar_2(n2);

    double d_term = 1.0 / std::cosh(par(2) / abs_lim);
    d_term = d_term * d_term;  // square it
    for (int i = 0; i < n1; ++i) {
      dy1_dpar_0(i) = x1[i];
      dy1_dpar_2(i) = d_term;
    }
    for (int i = 0; i < n2; ++i) {
      dy2_dpar_1(i) = x2[i];
      dy2_dpar_2(i) = d_term;
    }

    // Concatenate the derivatives for both tracks into full vectors of length
    // n.
    Eigen::VectorXd dy_dpar_0(n), dy_dpar_1(n), dy_dpar_2(n);
    for (int i = 0; i < n1; ++i) {
      dy_dpar_0(i) = dy1_dpar_0(i);
      dy_dpar_1(i) = dy1_dpar_1(i);
      dy_dpar_2(i) = dy1_dpar_2(i);
    }
    for (int i = 0; i < n2; ++i) {
      dy_dpar_0(n1 + i) = dy2_dpar_0(i);
      dy_dpar_1(n1 + i) = dy2_dpar_1(i);
      dy_dpar_2(n1 + i) = dy2_dpar_2(i);
    }

    // Build the "A" matrix (the Jacobian) in its transposed form (3 x n)
    Eigen::MatrixXd a_trans(3, n);
    a_trans.row(0) = dy_dpar_0.transpose();
    a_trans.row(1) = dy_dpar_1.transpose();
    a_trans.row(2) = dy_dpar_2.transpose();

    // The Jacobian (n x 3) is the transpose of a_trans.
    Eigen::MatrixXd a = a_trans.transpose();

    // The residual vector (difference between measured and fitted y values)
    Eigen::VectorXd dy_vec = y - y_fit;

    // Compute the (3 x 3) matrix: M = a_trans * W * a
    Eigen::MatrixXd temp = a_trans * W;  // 3 x n
    Eigen::MatrixXd temp2 = temp * a;    // 3 x 3

    // Add a regularization term to ensure numerical stability.
    Eigen::MatrixXd reg =
        1e-10 * Eigen::MatrixXd::Identity(temp2.rows(), temp2.cols());
    Eigen::MatrixXd temp2_reg = temp2 + reg;

    // Invert the matrix to obtain the covariance matrix.
    cov = temp2_reg.inverse();

    // Compute the parameter correction: dpar = cov * a_trans * W * dy_vec
    Eigen::MatrixXd temp4 = cov * a_trans;  // 3 x n
    Eigen::MatrixXd temp5 = temp4 * W;      // 3 x n
    Eigen::VectorXd dpar = temp5 * dy_vec;  // 3 x 1

    // Update the parameters
    par += dpar;

    // After the update, the fitted y values are recalculated with a different
    // formula:
    //   y1_fit = par[0]*(x1 - par[2])
    //   y2_fit = par[1]*(x2 - par[2])
    for (int i = 0; i < n1; ++i) {
      y1_fit(i) = par(0) * (x1[i] - par(2));
    }
    for (int i = 0; i < n2; ++i) {
      y2_fit(i) = par(1) * (x2[i] - par(2));
    }
    for (int i = 0; i < n1; ++i) {
      y_fit(i) = y1_fit(i);
    }
    for (int i = 0; i < n2; ++i) {
      y_fit(n1 + i) = y2_fit(i);
    }

    // Recompute chi-squared with the updated fitted values.
    double new_chi_sq = 0.0;
    for (int i = 0; i < n; ++i) {
      double diff = y_fit(i) - y(i);
      new_chi_sq += (diff * diff) / (s(i) * s(i));
    }
    chi_sq = new_chi_sq;

    // Check for convergence
    if (iter > 0) {
      if (std::abs(chi_sq - old_chi_sq) < d_chisq) {
        break;
      }
    }
    old_chi_sq = chi_sq;
  }  // end for loop

  if (verbose > 0) {
    ldmx_log(debug) << "At the end chi_sq = " << chi_sq;
    ldmx_log(debug) << "Scaled residuals for track 1:";
    for (int i = 0; i < n1; ++i) {
      double fit_val = par(0) * (x1[i] - par(2));
      ldmx_log(debug) << 10000 * (fit_val - y1[i]);
    }
    ldmx_log(debug) << "Scaled residuals for track 2:";
    for (int i = 0; i < n2; ++i) {
      double fit_val = par(1) * (x2[i] - par(2));
      ldmx_log(debug) << 10000 * (fit_val - y2[i]);
    }
  }

  int ndof = n1 + n2 - 3;
  return std::make_tuple(par, chi_sq, ndof, cov, n_iter);
}

std::pair<Eigen::VectorXd, Eigen::VectorXd> EcalWABRecProcessor::polyfitXYvsZ(
    const std::vector<double>& x, const std::vector<double>& y,
    const std::vector<double>& z, int degree) {
  /*
    Function that fits two polynomials (x vs. z and y vs. z) to 3D hit position
    data using a least-squares method. The fitted models are defined as: x = a₀
    + a₁ * z + a₂ * z² + ... + aₙ * zⁿ y = b₀ + b₁ * z + b₂ * z² + ... + bₙ * zⁿ
    where n is the specified polynomial degree.

    Inputs:
      x, y, z : measured coordinates for the tracks;
                x and y are the dependent variables, and z is the independent
    variable (all provided as std::vector<double>) degree  : degree of the
    polynomial to be fitted (int)

    Returns:
      A pair containing:
        first  : polynomial coefficients for the x vs. z fit (Eigen::VectorXd)
        second : polynomial coefficients for the y vs. z fit (Eigen::VectorXd)

    Notes:
      The polynomial is represented with the constant term first (i.e., [a₀, a₁,
    ..., aₙ]), so the linear term (slope) is located at index 1.
  */
  const size_t n = z.size();
  if (n == 0 || x.size() != n || y.size() != n) {
    throw std::invalid_argument(
        "Vectors x, y, and z must be non-empty and have the same size.");
  }

  // Construct the Vandermonde (design) matrix A (n x (degree + 1)):
  // Each row i: [1, z[i], z[i]^2, ..., z[i]^degree]
  Eigen::MatrixXd A(n, degree + 1);
  for (size_t i = 0; i < n; ++i) {
    double term = 1.0;
    for (int j = 0; j <= degree; ++j) {
      A(i, j) = term;
      term *= z[i];
    }
  }

  // Map the x and y data into Eigen vectors.
  Eigen::VectorXd bx(n), by(n);
  for (size_t i = 0; i < n; ++i) {
    bx(i) = x[i];
    by(i) = y[i];
  }

  // Solve the least-squares problems:
  // A * coeffsX ≈ bx and A * coeffsY ≈ by
  Eigen::VectorXd coeffsX = A.colPivHouseholderQr().solve(bx);
  Eigen::VectorXd coeffsY = A.colPivHouseholderQr().solve(by);

  return {coeffsX, coeffsY};
}
}  // namespace ecal

DECLARE_PRODUCER_NS(ecal, EcalWABRecProcessor);