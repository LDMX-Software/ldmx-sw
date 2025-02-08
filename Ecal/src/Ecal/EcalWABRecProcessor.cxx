#include "Ecal/EcalWABRecProcessor.h"

// LDMX
#include "DetDescr/SimSpecialID.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "SimCore/Event/SimParticle.h"
#include "Ecal/Event/EcalHit.h"
#include "Recon/Event/EventConstants.h"

/*~~~~~~~~~~~*/
/*   Tools   */
/*~~~~~~~~~~~*/
#include "Tools/AnalysisUtils.h"

// C++
#include <stdlib.h>
#include <vector>
#include <tuple>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <numeric>
#include <numbers> // For std::numbers::pi

// ROOT (MIP tracking)
#include "TMatrixD.h"
#include "TDecompSVD.h"
#include "TVector3.h"
// #include "Eigen/Dense"

namespace ecal {

const std::vector<double> radius68_thet0to10 = {
    10.12233413, 9.921772, 11.38255086, 11.67991867, 13.14337347, 13.17120624,
    16.80994665, 17.83787244, 22.44684374, 23.74239886, 28.60564083, 30.27889678,
    34.86404888, 36.39009394, 41.29309474, 43.34682279, 48.55982854, 50.80565589,
    55.29496257, 57.92737879, 60.64828824, 65.51760517, 68.26709803, 76.32877518,
    84.61219467, 98.21320491, 110.9880892, 120.6762931, 140.6174478, 136.4979268,
    145.579465, 154.9803228, 164.7005, 174.7399968
};

const std::vector<double> radius68_thet10to15 = {
    10.82307758, 11.17850518, 16.2185281, 18.62488713, 22.63408229, 24.71769042,
    30.11217538, 32.69939046, 37.99753196, 40.81619543, 45.89054775, 49.03066318,
    54.00440948, 59.31733555, 63.40789682, 64.77580021, 73.00113678, 73.25561396,
    78.8914776, 86.73962133, 97.05926327, 96.6932739, 111.6226151, 106.5960265,
    109.477541, 128.0220711, 145.4137195, 210.3582819, 199.6355662, 184.2513208,
    195.076552, 206.2322029, 217.7182737, 229.5347642
};

const std::vector<double> radius68_thet15to20 = {
    12.79450901, 13.02698578, 21.27450933, 25.66008312, 31.78592103, 35.99689874,
    44.37101115, 48.82709363, 55.05972458, 59.68948687, 65.39866214, 70.59280337,
    76.06007787, 82.22695257, 87.50371819, 90.60099831, 96.34848268, 101.4928478,
    106.7157092, 105.0540604, 110.0653355, 148.3428736, 133.1449443, 146.997265,
    173.3954389, 175.4329544, 184.3003543, 259.8415751, 215.0165, 231.1288534,
    243.4440277, 256.085458, 269.0531444, 282.3470868
};

const std::vector<double> radius68_thet20to30 = {
    14.16989595, 15.4488322, 28.31044668, 37.54285657, 48.57288885, 57.04243339,
    68.99836079, 75.33388728, 85.00572867, 91.52574074, 102.5044698, 106.5315986,
    116.2341378, 127.1121442, 133.8866375, 144.5121759, 162.1726963, 160.2986579,
    171.386638, 182.5653112, 205.5853241, 196.3113071, 200.5907513, 228.7275694,
    234.0298491, 251.7701385, 293.9351568, 310.521898, 344.1455457, 293.3518953,
    303.2401036, 313.128312, 323.0165203, 332.9047287
};

const std::vector<double> radius68_thet30to90 = {
    22.50983127, 26.44537503, 58.24642887, 90.59076279, 130.0592014, 157.4611392,
    184.2187293, 202.6994588, 225.3488816, 243.3454167, 269.2456428, 280.6119298,
    303.8591523, 322.0522722, 335.1780181, 350.3398234, 353.7763544, 373.9942362,
    382.9453608, 401.9703438, 441.6281859, 432.5241826, 455.2878243, 492.2888656,
    502.6653722, 480.2334627, 566.5438302, 505.7032783, 556.1650321, 596.9112032,
    616.1614593, 635.4117154, 654.6619715, 673.9122276
};

void EcalWABRecProcessor::configure(framework::config::Parameters &parameters) {
  // Set the collection name as defined in the configuration
  collectionName_ = parameters.getParameter<std::string>("collection_name");
  rec_pass_name_ = parameters.getParameter<std::string>("rec_pass_name");
  rec_coll_name_ = parameters.getParameter<std::string>("rec_coll_name");
  track_pass_name_ = parameters.getParameter<std::string>("track_pass_name");
  track_coll_name_ = parameters.getParameter<std::string>("rack_coll_name");
}

void EcalWABRecProcessor::produce(framework::Event &event) {
  // Get the Ecal Geometry
  geometry_ = &getCondition<ldmx::EcalGeometry>(
      ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  // Get the collection of digitized Ecal hits from the event
  const std::vector<ldmx::EcalHit> ecalRecHits =
      event.getCollection<ldmx::EcalHit>(rec_coll_name_, rec_pass_name_);

  const std::vector<ldmx::EcalHit> linearTracks =
      event.getCollection<ldmx::EcalHit>(track_coll_name_, track_pass_name_);

  std::vector<double> recoilE_P;
  std::vector<float> recoilE_Pos;
  std::vector<double> recoilY_P;
  std::vector<float> recoilY_Pos;
  
  // result object that stores kinematic variables
  ldmx::EcalWABResult result;

  // defining variables
  double trueThetaElectron = -9.;
  double trueThetaPhoton = -9.;
  double truePhiElectron = -9.;
  double truePhiPhoton = -9.;
  double recThetaElectron = -9.;
  double recThetaPhoton = -9.;
  double recPhiElectron = -9.;
  double recPhiPhoton = -9.;
  double trueThetaDiffElectronPhoton = -9.;
  double truePhiDiffElectronPhoton = -9.;
  double recThetaDiffElectronPhoton = -9.;
  double recPhiDiffElectronPhoton = -9.;
  double trueRecThetaDiffElectron = -9.;
  double trueRecPhiDiffElectron = -9.;
  double trueRecThetaDiffPhoton = -9.;
  double trueRecPhiDiffPhoton = -9.;

  // creating lists to save rec/tsp hits
  std::vector<std::array<double, 3>> recHitList;

  // save rec hit positions to recHitList
  for (const ldmx::EcalHit &hit : ecalRecHits) {
    ldmx::EcalID id(hit.getID());
    auto [x,y,z] = geometry_->getPosition(id);
    recHitList.push_back({x, y, z});
  }

  if (event.exists("TargetScoringPlaneHits")) {
    //
    // Loop through all of the sim particles and find the recoil photon/electron.
    //

    // Get the collection of simulated particles from the event
    auto particleMap{event.getMap<int, ldmx::SimParticle>("SimParticles")};

    // Search for the recoil electron
    auto [recoilTrackID, recoilElectron] = Analysis::getRecoil(particleMap);

    // Find Target SP hit for recoil photon/electron
    std::vector<ldmx::SimTrackerHit> targetSpHits =
          event.getCollection<ldmx::SimTrackerHit>("TargetScoringPlaneHits");
    float pmax = 0;
    for (ldmx::SimTrackerHit &spHit : targetSpHits) {
      ldmx::SimSpecialID hit_id(spHit.getID());
      if (hit_id.plane() != 1 || spHit.getMomentum()[2] <= 0) continue;

      if (spHit.getTrackID() == recoilTrackID) {
          recoilE_P = spHit.getMomentum();
          recoilE_Pos = spHit.getPosition();
      }
      if (spHit.getPdgID() == 22) {
        if (sqrt(pow(spHit.getMomentum()[0], 2) +
                  pow(spHit.getMomentum()[1], 2) +
                  pow(spHit.getMomentum()[2], 2)) > pmax) {
          recoilY_P = spHit.getMomentum();
          recoilY_Pos = spHit.getPosition();
          pmax =
              sqrt(pow(recoilY_P[0], 2) + pow(recoilY_P[1], 2) +
                    pow(recoilY_P[2], 2));
        }
      }
    }

    // Calculating true theta values using SP hit parameters
    std::vector<double> thetaVec = {0, 0, 1};
    if (recoilY_P.size() == 3 && sqrt(pow(recoilY_P[0], 2) + pow(recoilY_P[1], 2) + pow(recoilY_P[2], 2)) != 0 && recoilE_P.size() == 3 && sqrt(pow(recoilE_P[0], 2) + pow(recoilE_P[1], 2) + pow(recoilE_P[2], 2)) != 0) {
      trueThetaElectron = (180/std::numbers::pi)*std::acos(std::inner_product(recoilE_P.begin(), recoilE_P.end(), thetaVec.begin(), 0.0)/sqrt(pow(recoilE_P[0], 2) + pow(recoilE_P[1], 2) + pow(recoilE_P[2], 2)));
      trueThetaPhoton = (180/std::numbers::pi)*std::acos(std::inner_product(recoilY_P.begin(), recoilY_P.end(), thetaVec.begin(), 0.0)/sqrt(pow(recoilY_P[0], 2) + pow(recoilY_P[1], 2) + pow(recoilY_P[2], 2)));
    }

    // Calculating true phi values using SP hit parameters
    if (recoilY_P.size() == 3 && recoilY_P[0] != 0 && recoilE_P.size() == 3 && recoilE_P[0] != 0) {
      truePhiElectron = (180/std::numbers::pi)*std::atan(recoilE_P[1]/recoilE_P[0]);
      if (recoilE_P[1] < 0) {truePhiElectron += 180;}
      if (recoilE_P[0] < 0 && recoilE_P[1] > 0) {truePhiElectron += 360;}
      truePhiPhoton = (180/std::numbers::pi)*std::atan(recoilY_P[1]/recoilY_P[0]);
      if (recoilY_P[1] < 0) {truePhiPhoton += 180;}
      if (recoilY_P[0] < 0 && recoilY_P[1] > 0) {truePhiPhoton += 360;}
    }
    
    // Calculating true delta_phi/delta_theta using SP hit parameters
    if (recoilY_P.size() == 3 && recoilE_P.size() == 3) {
      std::array<double, 2> phiDiffElectronArr = {recoilE_P[0], recoilE_P[1]};
      std::array<double, 2> phiDiffPhotonArr = {recoilY_P[0], recoilY_P[1]};
      std::array<double, 2> thetaDiffElectronArr = {recoilE_P[2], recoilE_P[0]};
      std::array<double, 2> thetaDiffPhotonArr = {recoilY_P[2], recoilY_P[0]};
      
      trueThetaDiffElectronPhoton = (180/std::numbers::pi)*std::acos(std::inner_product(thetaDiffElectronArr.begin(), thetaDiffElectronArr.end(), thetaDiffPhotonArr.begin(), 0.0)/(sqrt(pow(thetaDiffElectronArr[0], 2) + pow(thetaDiffElectronArr[1], 2)) * sqrt(pow(thetaDiffPhotonArr[0], 2) + pow(thetaDiffPhotonArr[1], 2))));
      truePhiDiffElectronPhoton = (180/std::numbers::pi)*std::acos(std::inner_product(phiDiffElectronArr.begin(), phiDiffElectronArr.end(), phiDiffPhotonArr.begin(), 0.0)/(sqrt(pow(phiDiffElectronArr[0], 2) + pow(phiDiffElectronArr[1], 2)) * sqrt(pow(phiDiffPhotonArr[0], 2) + pow(phiDiffPhotonArr[1], 2))));
    }
  }

  

  // Setting output object equal to calculated variables
  result.setVariables(trueThetaElectron, trueThetaPhoton, truePhiElectron, truePhiPhoton, recThetaElectron, 
        recThetaPhoton, recPhiElectron, recPhiPhoton, trueThetaDiffElectronPhoton, truePhiDiffElectronPhoton, 
        recThetaDiffElectronPhoton, recPhiDiffElectronPhoton, trueRecThetaDiffElectron, trueRecPhiDiffElectron, 
        trueRecThetaDiffPhoton, trueRecPhiDiffPhoton);

  event.add(collectionName_, result);
}

// // The function fits two 2D tracks constrained by a common parameter.
// // The fitted model is initially defined as:
// //    y1 = par[0] * x1 + abs_lim * tanh(par[2] / abs_lim)
// //    y2 = par[1] * x2 + abs_lim * tanh(par[2] / abs_lim)
// // After updating parameters the fitted values are computed as:
// //    y1 = par[0] * (x1 - par[2])
// //    y2 = par[1] * (x2 - par[2])
// //
// // Inputs:
// //   x1, y1, s1 : measured coordinates and errors for track 1
// //   x2, y2, s2 : measured coordinates and errors for track 2
// //   guess     : initial guess for the parameter vector (size 3)
// //   maxIter   : maximum number of iterations (default 20)
// //   verbosity : level of verbosity (default 0)
// //   dchisq    : stopping criterion for chi-squared improvement (default 0.001)
// //   abs_lim   : a parameter used in the fit (default 10)
// // Returns:
// //   A tuple containing:
// //     par    : fitted parameters (Eigen::VectorXd of size 3)
// //     chisq  : chi-squared at minimum (double)
// //     ndof   : number of degrees of freedom (int)
// //     cov    : covariance matrix (Eigen::MatrixXd 3x3)
// //     niter  : number of iterations used (int)
// void std::tuple<Eigen::VectorXd, double, int, Eigen::MatrixXd, int>
// fit2DTracksConstrained(const std::vector<double>& x1,
//                          const std::vector<double>& y1,
//                          const std::vector<double>& s1,
//                          const std::vector<double>& x2,
//                          const std::vector<double>& y2,
//                          const std::vector<double>& s2,
//                          const std::vector<double>& guess,
//                          int maxIter = 20,
//                          int verbosity = 0,
//                          double dchisq = 0.001,
//                          double abs_lim = 10.0)
// {
//     // Copy the initial guess into a 3-element parameter vector
//     Eigen::VectorXd par(3);
//     par(0) = guess[0];
//     par(1) = guess[1];
//     par(2) = guess[2];

//     // Determine number of points in each track and total
//     int n1 = x1.size();
//     int n2 = x2.size();
//     int n  = n1 + n2;

//     // Concatenate x, y, and s into Eigen vectors of size n.
//     Eigen::VectorXd x(n), y(n), s(n);
//     for (int i = 0; i < n1; ++i) {
//         x(i) = x1[i];
//         y(i) = y1[i];
//         s(i) = s1[i];
//     }
//     for (int i = 0; i < n2; ++i) {
//         x(n1 + i) = x2[i];
//         y(n1 + i) = y2[i];
//         s(n1 + i) = s2[i];
//     }

//     // Build the weight matrix W = diag(1/s_i^2)
//     Eigen::MatrixXd W = Eigen::MatrixXd::Zero(n, n);
//     for (int i = 0; i < n; ++i) {
//         W(i, i) = 1.0 / (s(i) * s(i));
//     }

//     double chisq   = 0.0;
//     double oldchisq = 1e12;  // a large initial value
//     int niter = 0;
//     Eigen::MatrixXd cov(3, 3);  // covariance matrix

//     // Iterative fitting loop
//     for (int iter = 0; iter < maxIter; ++iter) {
//         niter = iter + 1;

//         // Compute fitted y coordinates for each track using the current parameters.
//         // For track 1: y1fit = par[0] * x1 + abs_lim * tanh(par[2]/abs_lim)
//         // For track 2: y2fit = par[1] * x2 + abs_lim * tanh(par[2]/abs_lim)
//         Eigen::VectorXd y1fit(n1), y2fit(n2);
//         double tanhTerm = std::tanh(par(2) / abs_lim);
//         for (int i = 0; i < n1; ++i) {
//             y1fit(i) = par(0) * x1[i] + abs_lim * tanhTerm;
//         }
//         for (int i = 0; i < n2; ++i) {
//             y2fit(i) = par(1) * x2[i] + abs_lim * tanhTerm;
//         }

//         // Concatenate the fitted values
//         Eigen::VectorXd yfit(n);
//         for (int i = 0; i < n1; ++i) {
//             yfit(i) = y1fit(i);
//         }
//         for (int i = 0; i < n2; ++i) {
//             yfit(n1 + i) = y2fit(i);
//         }

//         // Compute chi-squared: sum_i [ (yfit[i]-y[i])^2 / s[i]^2 ]
//         chisq = 0.0;
//         for (int i = 0; i < n; ++i) {
//             double diff = yfit(i) - y(i);
//             chisq += (diff * diff) / (s(i) * s(i));
//         }

//         if (verbosity > 0) {
//             std::cout << "Before iteration " << iter << ", chisq = " << chisq << std::endl;
//             std::cout << "Track 1 residuals: ";
//             for (int i = 0; i < n1; ++i) {
//                 std::cout << (y1fit(i) - y1[i]) << " ";
//             }
//             std::cout << std::endl;
//             std::cout << "Track 2 residuals: ";
//             for (int i = 0; i < n2; ++i) {
//                 std::cout << (y2fit(i) - y2[i]) << " ";
//             }
//             std::cout << std::endl;
//         }

//         // Compute the derivatives (Jacobian components)
//         // For track 1:
//         //   dy1/dpar0 = x1,   dy1/dpar1 = 0,   dy1/dpar2 = (1/cosh(par[2]/abs_lim))^2 (constant for all points)
//         // For track 2:
//         //   dy2/dpar0 = 0,   dy2/dpar1 = x2,   dy2/dpar2 = (1/cosh(par[2]/abs_lim))^2
//         Eigen::VectorXd dy1_dpar0(n1), dy1_dpar1 = Eigen::VectorXd::Zero(n1), dy1_dpar2(n1);
//         Eigen::VectorXd dy2_dpar0 = Eigen::VectorXd::Zero(n2), dy2_dpar1(n2), dy2_dpar2(n2);

//         double d_term = 1.0 / std::cosh(par(2) / abs_lim);
//         d_term = d_term * d_term;  // square it
//         for (int i = 0; i < n1; ++i) {
//             dy1_dpar0(i) = x1[i];
//             dy1_dpar2(i) = d_term;
//         }
//         for (int i = 0; i < n2; ++i) {
//             dy2_dpar1(i) = x2[i];
//             dy2_dpar2(i) = d_term;
//         }

//         // Concatenate the derivatives for both tracks into full vectors of length n.
//         Eigen::VectorXd dy_dpar0(n), dy_dpar1(n), dy_dpar2(n);
//         for (int i = 0; i < n1; ++i) {
//             dy_dpar0(i) = dy1_dpar0(i);
//             dy_dpar1(i) = dy1_dpar1(i);
//             dy_dpar2(i) = dy1_dpar2(i);
//         }
//         for (int i = 0; i < n2; ++i) {
//             dy_dpar0(n1 + i) = dy2_dpar0(i);
//             dy_dpar1(n1 + i) = dy2_dpar1(i);
//             dy_dpar2(n1 + i) = dy2_dpar2(i);
//         }

//         // Build the "A" matrix (the Jacobian) in its transposed form (3 x n)
//         Eigen::MatrixXd Atrans(3, n);
//         Atrans.row(0) = dy_dpar0.transpose();
//         Atrans.row(1) = dy_dpar1.transpose();
//         Atrans.row(2) = dy_dpar2.transpose();

//         // The Jacobian (n x 3) is the transpose of Atrans.
//         Eigen::MatrixXd A = Atrans.transpose();

//         // The residual vector (difference between measured and fitted y values)
//         Eigen::VectorXd dy_vec = y - yfit;

//         // Compute the (3 x 3) matrix: M = Atrans * W * A
//         Eigen::MatrixXd temp = Atrans * W;  // 3 x n
//         Eigen::MatrixXd temp2 = temp * A;     // 3 x 3

//         // Add a regularization term to ensure numerical stability.
//         Eigen::MatrixXd reg = 1e-10 * Eigen::MatrixXd::Identity(temp2.rows(), temp2.cols());
//         Eigen::MatrixXd temp2_reg = temp2 + reg;

//         // Invert the matrix to obtain the covariance matrix.
//         cov = temp2_reg.inverse();

//         // Compute the parameter correction: dpar = cov * Atrans * W * dy_vec
//         Eigen::MatrixXd temp4 = cov * Atrans; // 3 x n
//         Eigen::MatrixXd temp5 = temp4 * W;      // 3 x n
//         Eigen::VectorXd dpar = temp5 * dy_vec;  // 3 x 1

//         // Update the parameters
//         par += dpar;

//         // After the update, the fitted y values are recalculated with a different formula:
//         //   y1fit = par[0]*(x1 - par[2])
//         //   y2fit = par[1]*(x2 - par[2])
//         for (int i = 0; i < n1; ++i) {
//             y1fit(i) = par(0) * (x1[i] - par(2));
//         }
//         for (int i = 0; i < n2; ++i) {
//             y2fit(i) = par(1) * (x2[i] - par(2));
//         }
//         for (int i = 0; i < n1; ++i) {
//             yfit(i) = y1fit(i);
//         }
//         for (int i = 0; i < n2; ++i) {
//             yfit(n1 + i) = y2fit(i);
//         }

//         // Recompute chi-squared with the updated fitted values.
//         double new_chisq = 0.0;
//         for (int i = 0; i < n; ++i) {
//             double diff = yfit(i) - y(i);
//             new_chisq += (diff * diff) / (s(i) * s(i));
//         }
//         chisq = new_chisq;

//         // Check for convergence
//         if (iter > 0) {
//             if (std::abs(chisq - oldchisq) < dchisq) {
//                 break;
//             }
//         }
//         oldchisq = chisq;
//     } // end for loop

//     if (verbosity > 0) {
//         std::cout << "At the end chisq = " << chisq << std::endl;
//         std::cout << "Scaled residuals for track 1:" << std::endl;
//         for (int i = 0; i < n1; ++i) {
//             double fit_val = par(0) * (x1[i] - par(2));
//             std::cout << 10000 * (fit_val - y1[i]) << " ";
//         }
//         std::cout << std::endl;
//         std::cout << "Scaled residuals for track 2:" << std::endl;
//         for (int i = 0; i < n2; ++i) {
//             double fit_val = par(1) * (x2[i] - par(2));
//             std::cout << 10000 * (fit_val - y2[i]) << " ";
//         }
//         std::cout << std::endl;
//     }

//     int ndof = n1 + n2 - 3;
//     return std::make_tuple(par, chisq, ndof, cov, niter);
// }

}  // namespace ecal

DECLARE_PRODUCER_NS(ecal, EcalWABRecProcessor);
