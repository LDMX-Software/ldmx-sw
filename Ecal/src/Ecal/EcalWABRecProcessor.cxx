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
#include <algorithm>
#include <cmath>
#include <fstream>
#include <numeric>
#include <numbers> // For std::numbers::pi

// ROOT (MIP tracking)
#include "TMatrixD.h"
#include "TDecompSVD.h"
#include "TVector3.h"

namespace ecal {

void EcalWABRecProcessor::configure(framework::config::Parameters &parameters) {
  // Set the collection name as defined in the configuration
  collectionName_ = parameters.getParameter<std::string>("collection_name");
  rec_pass_name_ = parameters.getParameter<std::string>("rec_pass_name");
  rec_coll_name_ = parameters.getParameter<std::string>("rec_coll_name");
}

void EcalWABRecProcessor::produce(framework::Event &event) {
  // Get the Ecal Geometry
  geometry_ = &getCondition<ldmx::EcalGeometry>(
      ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  // Get the collection of digitized Ecal hits from the event
  const std::vector<ldmx::EcalHit> ecalRecHits =
      event.getCollection<ldmx::EcalHit>(rec_coll_name_, rec_pass_name_);

  std::vector<double> recoilE_P;
  std::vector<float> recoilE_Pos;
  std::vector<double> recoilY_P;
  std::vector<float> recoilY_Pos;
  
  // result object that stores kinematic variables
  ldmx::EcalWABResult result;

  // defining variables
  double trueThetaElectron = -999.;
  double trueThetaPhoton = -999.;
  double truePhiElectron = -999.;
  double truePhiPhoton = -999.;
  double recThetaElectron = -999.;
  double recThetaPhoton = -999.;
  double recPhiElectron = -999.;
  double recPhiPhoton = -999.;
  double trueThetaDiffElectronPhoton = -999.;
  double truePhiDiffElectronPhoton = -999.;
  double recThetaDiffElectronPhoton = -999.;
  double recPhiDiffElectronPhoton = -999.;
  double trueRecThetaDiffElectron = -999.;
  double trueRecPhiDiffElectron = -999.;
  double trueRecThetaDiffPhoton = -999.;
  double trueRecPhiDiffPhoton = -999.;

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
    trueThetaElectron = (180/std::numbers::pi)*std::acos(std::inner_product(recoilE_P.begin(), recoilE_P.end(), thetaVec.begin(), 0.0)/sqrt(pow(recoilE_P[0], 2) + pow(recoilE_P[1], 2) + pow(recoilE_P[2], 2)));
    if (recoilY_P.size() == 3 && sqrt(pow(recoilY_P[0], 2) + pow(recoilY_P[1], 2) + pow(recoilY_P[2], 2)) != 0) {
      trueThetaPhoton = (180/std::numbers::pi)*std::acos(std::inner_product(recoilY_P.begin(), recoilY_P.end(), thetaVec.begin(), 0.0)/sqrt(pow(recoilY_P[0], 2) + pow(recoilY_P[1], 2) + pow(recoilY_P[2], 2)));
    }

    // Calculating true phi values using SP hit parameters
    truePhiElectron = (180/std::numbers::pi)*std::atan(recoilE_P[1]/recoilE_P[0]);
    if (recoilE_P[1] < 0) {truePhiElectron += 180;}
    if (recoilE_P[0] < 0 && recoilE_P[1] > 0) {truePhiElectron += 360;}
    
    if (recoilY_P.size() == 3 && recoilY_P[0] != 0) {
      truePhiPhoton = (180/std::numbers::pi)*std::atan(recoilY_P[1]/recoilY_P[0]);
      if (recoilY_P[1] < 0) {truePhiPhoton += 180;}
      if (recoilY_P[0] < 0 && recoilY_P[1] > 0) {truePhiPhoton += 360;}
    }

    if (recoilY_P.size() == 3) {
      std::array<double, 2> phiDiffElectronArr = {recoilE_P[0], recoilE_P[1]};
      std::array<double, 2> phiDiffPhotonArr = {recoilY_P[0], recoilY_P[1]};
      std::array<double, 2> thetaDiffElectronArr = {recoilE_P[2], recoilE_P[0]};
      std::array<double, 2> thetaDiffPhotonArr = {recoilY_P[2], recoilY_P[0]};
      
      trueThetaDiffElectronPhoton = (180/std::numbers::pi)*std::acos(std::inner_product(thetaDiffElectronArr.begin(), thetaDiffElectronArr.end(), thetaDiffPhotonArr.begin(), 0.0)/(sqrt(pow(thetaDiffElectronArr[0], 2) + pow(thetaDiffElectronArr[1], 2)) * sqrt(pow(thetaDiffPhotonArr[0], 2) + pow(thetaDiffPhotonArr[1], 2))));
      truePhiDiffElectronPhoton = (180/std::numbers::pi)*std::acos(std::inner_product(phiDiffElectronArr.begin(), phiDiffElectronArr.end(), phiDiffPhotonArr.begin(), 0.0)/(sqrt(pow(phiDiffElectronArr[0], 2) + pow(phiDiffElectronArr[1], 2)) * sqrt(pow(phiDiffPhotonArr[0], 2) + pow(phiDiffPhotonArr[1], 2))));
    }
  }
  
  result.setVariables(trueThetaElectron, trueThetaPhoton, truePhiElectron, truePhiPhoton, recThetaElectron, 
        recThetaPhoton, recPhiElectron, recPhiPhoton, trueThetaDiffElectronPhoton, truePhiDiffElectronPhoton, 
        recThetaDiffElectronPhoton, recPhiDiffElectronPhoton, trueRecThetaDiffElectron, trueRecPhiDiffElectron, 
        trueRecThetaDiffPhoton, trueRecPhiDiffPhoton);

  event.add(collectionName_, result);
}

}  // namespace ecal

DECLARE_PRODUCER_NS(ecal, EcalWABRecProcessor);
