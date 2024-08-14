#include "Ecal/EcalVetoProcessor.h"

// LDMX
#include "DetDescr/SimSpecialID.h"
#include "Ecal/Event/EcalHit.h"
#include "Recon/Event/EventConstants.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"

/*~~~~~~~~~~~*/
/*   Tools   */
/*~~~~~~~~~~~*/
#include "Tools/AnalysisUtils.h"

// C++
#include <stdlib.h>

#include <algorithm>
#include <cmath>
#include <fstream>

// ROOT (MIP tracking)
#include "TDecompSVD.h"
#include "TMatrixD.h"
#include "TVector3.h"

namespace ecal {

void EcalVetoProcessor::buildBDTFeatureVector(
    const ldmx::EcalVetoResult &result) {
  // Base variables
  bdtFeatures_.push_back(result.getNReadoutHits());
  bdtFeatures_.push_back(result.getSummedDet());
  bdtFeatures_.push_back(result.getSummedTightIso());
  bdtFeatures_.push_back(result.getMaxCellDep());
  bdtFeatures_.push_back(result.getShowerRMS());
  bdtFeatures_.push_back(result.getXStd());
  bdtFeatures_.push_back(result.getYStd());
  bdtFeatures_.push_back(result.getAvgLayerHit());
  bdtFeatures_.push_back(result.getStdLayerHit());
  bdtFeatures_.push_back(result.getDeepestLayerHit());
  bdtFeatures_.push_back(result.getEcalBackEnergy());
  // MIP tracking
  bdtFeatures_.push_back(result.getNStraightTracks());
  // bdtFeatures_.push_back(result.getNLinregTracks());
  bdtFeatures_.push_back(result.getFirstNearPhLayer());
  bdtFeatures_.push_back(result.getNNearPhHits());
  bdtFeatures_.push_back(result.getPhotonTerritoryHits());
  bdtFeatures_.push_back(result.getEPSep());
  bdtFeatures_.push_back(result.getEPDot());
  // Longitudinal segment variables
  bdtFeatures_.push_back(result.getEnergySeg()[0]);
  bdtFeatures_.push_back(result.getXMeanSeg()[0]);
  bdtFeatures_.push_back(result.getYMeanSeg()[0]);
  bdtFeatures_.push_back(result.getLayerMeanSeg()[0]);
  bdtFeatures_.push_back(result.getEnergySeg()[1]);
  bdtFeatures_.push_back(result.getYMeanSeg()[2]);
  /// Electron RoC variables
  bdtFeatures_.push_back(result.getEleContEnergy()[0][0]);
  bdtFeatures_.push_back(result.getEleContEnergy()[1][0]);
  bdtFeatures_.push_back(result.getEleContYMean()[0][0]);
  bdtFeatures_.push_back(result.getEleContEnergy()[0][1]);
  bdtFeatures_.push_back(result.getEleContEnergy()[1][1]);
  bdtFeatures_.push_back(result.getEleContYMean()[0][1]);
  /// Photon RoC variables
  bdtFeatures_.push_back(result.getPhContNHits()[0][0]);
  bdtFeatures_.push_back(result.getPhContYMean()[0][0]);
  bdtFeatures_.push_back(result.getPhContNHits()[0][1]);
  /// Outside RoC variables
  bdtFeatures_.push_back(result.getOutContEnergy()[0][0]);
  bdtFeatures_.push_back(result.getOutContEnergy()[1][0]);
  bdtFeatures_.push_back(result.getOutContEnergy()[2][0]);
  bdtFeatures_.push_back(result.getOutContNHits()[0][0]);
  bdtFeatures_.push_back(result.getOutContXMean()[0][0]);
  bdtFeatures_.push_back(result.getOutContYMean()[0][0]);
  bdtFeatures_.push_back(result.getOutContYMean()[1][0]);
  bdtFeatures_.push_back(result.getOutContYStd()[0][0]);
  bdtFeatures_.push_back(result.getOutContEnergy()[0][1]);
  bdtFeatures_.push_back(result.getOutContEnergy()[1][1]);
  bdtFeatures_.push_back(result.getOutContEnergy()[2][1]);
  bdtFeatures_.push_back(result.getOutContLayerMean()[0][1]);
  bdtFeatures_.push_back(result.getOutContLayerStd()[0][1]);
  bdtFeatures_.push_back(result.getOutContEnergy()[0][2]);
  bdtFeatures_.push_back(result.getOutContLayerMean()[0][2]);
}

void EcalVetoProcessor::configure(framework::config::Parameters &parameters) {
  doBdt_ = parameters.getParameter<bool>("do_bdt");
  featureListName_ = parameters.getParameter<std::string>("feature_list_name");
  if (doBdt_) {
    rt_ = std::make_unique<ldmx::Ort::ONNXRuntime>(
        parameters.getParameter<std::string>("bdt_file"));
  }

  cellFileNamexy_ = parameters.getParameter<std::string>("cellxy_file");
  if (!std::ifstream(cellFileNamexy_).good()) {
    EXCEPTION_RAISE("EcalVetoProcessor", "The specified x,y cell file '" +
                                             cellFileNamexy_ +
                                             "' does not exist!");
  } else {
    std::ifstream cellxyfile(cellFileNamexy_);
    float valuex;
    float valuey;
    while (cellxyfile >> valuex >> valuey) {
      mapsx.push_back(valuex);
      mapsy.push_back(valuey);
    }
  }

  // Read in arrays holding 68% containment radius per layer
  // for different bins in momentum/angle
  rocFileName_ = parameters.getParameter<std::string>("roc_file");
  if (!std::ifstream(rocFileName_).good()) {
    EXCEPTION_RAISE(
        "EcalVetoProcessor",
        "The specified RoC file '" + rocFileName_ + "' does not exist!");
  } else {
    std::ifstream rocfile(rocFileName_);
    std::string line, value;

    // Extract the first line in the file
    std::getline(rocfile, line);
    std::vector<double> values;

    // Read data, line by line
    while (std::getline(rocfile, line)) {
      std::stringstream ss(line);
      values.clear();
      while (std::getline(ss, value, ',')) {
        double f_value = (value != "") ? std::stof(value) : -1.0;
        values.push_back(f_value);
      }
      roc_range_values_.push_back(values);
    }
  }

  nEcalLayers_ = parameters.getParameter<int>("num_ecal_layers");

  bdtCutVal_ = parameters.getParameter<double>("disc_cut");
  ecalLayerEdepRaw_.resize(nEcalLayers_, 0);
  ecalLayerEdepReadout_.resize(nEcalLayers_, 0);
  ecalLayerTime_.resize(nEcalLayers_, 0);

  beamEnergyMeV_ = parameters.getParameter<double>("beam_energy");

  // Set the collection name as defined in the configuration
  collectionName_ = parameters.getParameter<std::string>("collection_name");
  rec_pass_name_ = parameters.getParameter<std::string>("rec_pass_name");
  rec_coll_name_ = parameters.getParameter<std::string>("rec_coll_name");
}

void EcalVetoProcessor::clearProcessor() {
  cellMap_.clear();
  cellMapTightIso_.clear();
  bdtFeatures_.clear();

  nReadoutHits_ = 0;
  summedDet_ = 0;
  summedTightIso_ = 0;
  maxCellDep_ = 0;
  showerRMS_ = 0;
  xStd_ = 0;
  yStd_ = 0;
  avgLayerHit_ = 0;
  stdLayerHit_ = 0;
  deepestLayerHit_ = 0;
  ecalBackEnergy_ = 0;
  // MIP tracking
  nStraightTracks_ = 0;
  nLinregTracks_ = 0;
  firstNearPhLayer_ = 0;
  nNearPhHits_ = 0;
  epAng_ = 0;
  epSep_ = 0;
  epDot_ = 0;
  photonTerritoryHits_ = 0;

  std::fill(ecalLayerEdepRaw_.begin(), ecalLayerEdepRaw_.end(), 0);
  std::fill(ecalLayerEdepReadout_.begin(), ecalLayerEdepReadout_.end(), 0);
  std::fill(ecalLayerTime_.begin(), ecalLayerTime_.end(), 0);
}

void EcalVetoProcessor::produce(framework::Event &event) {
  // Get the Ecal Geometry
  geometry_ = &getCondition<ldmx::EcalGeometry>(
      ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  ldmx::EcalVetoResult result;

  clearProcessor();

  // Get the collection of Ecal scoring plane hits. If it doesn't exist,
  // don't bother adding any truth tracking information.

  std::vector<double> recoilP;
  std::vector<float> recoilPos;
  std::vector<double> recoilPAtTarget;
  std::vector<float> recoilPosAtTarget;

  if (event.exists("EcalScoringPlaneHits")) {
    //
    // Loop through all of the sim particles and find the recoil electron.
    //

    // Get the collection of simulated particles from the event
    auto particleMap{event.getMap<int, ldmx::SimParticle>("SimParticles")};

    // Search for the recoil electron
    auto [recoilTrackID, recoilElectron] = Analysis::getRecoil(particleMap);

    // Find ECAL SP hit for recoil electron
    auto ecalSpHits{
        event.getCollection<ldmx::SimTrackerHit>("EcalScoringPlaneHits")};
    float pmax = 0;
    for (ldmx::SimTrackerHit &spHit : ecalSpHits) {
      ldmx::SimSpecialID hit_id(spHit.getID());
      if (hit_id.plane() != 31 || spHit.getMomentum()[2] <= 0) continue;

      if (spHit.getTrackID() == recoilTrackID) {
        if (sqrt(pow(spHit.getMomentum()[0], 2) +
                 pow(spHit.getMomentum()[1], 2) +
                 pow(spHit.getMomentum()[2], 2)) > pmax) {
          recoilP = spHit.getMomentum();
          recoilPos = spHit.getPosition();
          pmax = sqrt(pow(recoilP[0], 2) + pow(recoilP[1], 2) +
                      pow(recoilP[2], 2));
        }
      }
    }

    // Find target SP hit for recoil electron
    if (event.exists("TargetScoringPlaneHits")) {
      std::vector<ldmx::SimTrackerHit> targetSpHits =
          event.getCollection<ldmx::SimTrackerHit>("TargetScoringPlaneHits");
      pmax = 0;
      for (ldmx::SimTrackerHit &spHit : targetSpHits) {
        ldmx::SimSpecialID hit_id(spHit.getID());
        if (hit_id.plane() != 1 || spHit.getMomentum()[2] <= 0) continue;

        if (spHit.getTrackID() == recoilTrackID) {
          if (sqrt(pow(spHit.getMomentum()[0], 2) +
                   pow(spHit.getMomentum()[1], 2) +
                   pow(spHit.getMomentum()[2], 2)) > pmax) {
            recoilPAtTarget = spHit.getMomentum();
            recoilPosAtTarget = spHit.getPosition();
            pmax =
                sqrt(pow(recoilPAtTarget[0], 2) + pow(recoilPAtTarget[1], 2) +
                     pow(recoilPAtTarget[2], 2));
          }
        }
      }
    }
  }

  // Get projected trajectories for electron and photon
  std::vector<XYCoords> ele_trajectory, photon_trajectory;
  if (recoilP.size() > 0) {
    ele_trajectory = getTrajectory(recoilP, recoilPos);
    std::vector<double> pvec = recoilPAtTarget.size()
                                   ? recoilPAtTarget
                                   : std::vector<double>{0.0, 0.0, 0.0};
    std::vector<float> posvec = recoilPosAtTarget.size()
                                    ? recoilPosAtTarget
                                    : std::vector<float>{0.0, 0.0, 0.0};
    photon_trajectory =
        getTrajectory({-pvec[0], -pvec[1], beamEnergyMeV_ - pvec[2]}, posvec);
  }

  float recoilPMag =
      recoilP.size()
          ? sqrt(pow(recoilP[0], 2) + pow(recoilP[1], 2) + pow(recoilP[2], 2))
          : -1.0;
  float recoilTheta =
      recoilPMag > 0 ? acos(recoilP[2] / recoilPMag) * 180.0 / M_PI : -1.0;

  // Use the appropriate containment radii for the recoil electron
  std::vector<double> roc_values_bin0(roc_range_values_[0].begin() + 4,
                                      roc_range_values_[0].end());
  std::vector<double> ele_radii = roc_values_bin0;
  double theta_min, theta_max, p_min, p_max;
  bool inrange;

  for (int i = 0; i < roc_range_values_.size(); i++) {
    theta_min = roc_range_values_[i][0];
    theta_max = roc_range_values_[i][1];
    p_min = roc_range_values_[i][2];
    p_max = roc_range_values_[i][3];
    inrange = true;

    if (theta_min != -1.0) {
      inrange = inrange && (recoilTheta >= theta_min);
    }
    if (theta_max != -1.0) {
      inrange = inrange && (recoilTheta < theta_max);
    }
    if (p_min != -1.0) {
      inrange = inrange && (recoilPMag >= p_min);
    }
    if (p_max != -1.0) {
      inrange = inrange && (recoilPMag < p_max);
    }
    if (inrange) {
      std::vector<double> roc_values_bini(roc_range_values_[i].begin() + 4,
                                          roc_range_values_[i].end());
      ele_radii = roc_values_bini;
    }
  }
  // Use default RoC bin for photon
  std::vector<double> photon_radii = roc_values_bin0;

  // Get the collection of digitized Ecal hits from the event.
  const std::vector<ldmx::EcalHit> ecalRecHits =
      event.getCollection<ldmx::EcalHit>(rec_coll_name_, rec_pass_name_);

  ldmx::EcalID globalCentroid =
      GetShowerCentroidIDAndRMS(ecalRecHits, showerRMS_);
  /* ~~ Fill the hit map ~~ O(n)  */
  fillHitMap(ecalRecHits, cellMap_);
  bool doTight = true;
  /* ~~ Fill the isolated hit maps ~~ O(n)  */
  fillIsolatedHitMap(ecalRecHits, globalCentroid, cellMap_, cellMapTightIso_,
                     doTight);

  // Loop over the hits from the event to calculate the rest of the important
  // quantities

  float wavgLayerHit = 0;
  float xMean = 0;
  float yMean = 0;

  // Containment variables
  unsigned int nregions = 5;
  std::vector<float> electronContainmentEnergy(nregions, 0.0);
  std::vector<float> photonContainmentEnergy(nregions, 0.0);
  std::vector<float> outsideContainmentEnergy(nregions, 0.0);
  std::vector<int> outsideContainmentNHits(nregions, 0);
  std::vector<float> outsideContainmentXmean(nregions, 0.0);
  std::vector<float> outsideContainmentYmean(nregions, 0.0);
  std::vector<float> outsideContainmentXstd(nregions, 0.0);
  std::vector<float> outsideContainmentYstd(nregions, 0.0);
  // Longitudinal segmentation
  std::vector<int> segLayers = {0, 6, 17, 34};
  unsigned int nsegments = segLayers.size() - 1;
  std::vector<float> energySeg(nsegments, 0.0);
  std::vector<float> xMeanSeg(nsegments, 0.0);
  std::vector<float> xStdSeg(nsegments, 0.0);
  std::vector<float> yMeanSeg(nsegments, 0.0);
  std::vector<float> yStdSeg(nsegments, 0.0);
  std::vector<float> layerMeanSeg(nsegments, 0.0);
  std::vector<float> layerStdSeg(nsegments, 0.0);
  std::vector<std::vector<float>> eContEnergy(
      nregions, std::vector<float>(nsegments, 0.0));
  std::vector<std::vector<float>> eContXMean(
      nregions, std::vector<float>(nsegments, 0.0));
  std::vector<std::vector<float>> eContYMean(
      nregions, std::vector<float>(nsegments, 0.0));
  std::vector<std::vector<float>> gContEnergy(
      nregions, std::vector<float>(nsegments, 0.0));
  std::vector<std::vector<int>> gContNHits(nregions,
                                           std::vector<int>(nsegments, 0));
  std::vector<std::vector<float>> gContXMean(
      nregions, std::vector<float>(nsegments, 0.0));
  std::vector<std::vector<float>> gContYMean(
      nregions, std::vector<float>(nsegments, 0.0));
  std::vector<std::vector<float>> oContEnergy(
      nregions, std::vector<float>(nsegments, 0.0));
  std::vector<std::vector<int>> oContNHits(nregions,
                                           std::vector<int>(nsegments, 0));
  std::vector<std::vector<float>> oContXMean(
      nregions, std::vector<float>(nsegments, 0.0));
  std::vector<std::vector<float>> oContYMean(
      nregions, std::vector<float>(nsegments, 0.0));
  std::vector<std::vector<float>> oContXStd(nregions,
                                            std::vector<float>(nsegments, 0.0));
  std::vector<std::vector<float>> oContYStd(nregions,
                                            std::vector<float>(nsegments, 0.0));
  std::vector<std::vector<float>> oContLayerMean(
      nregions, std::vector<float>(nsegments, 0.0));
  std::vector<std::vector<float>> oContLayerStd(
      nregions, std::vector<float>(nsegments, 0.0));

  // MIP tracking:  vector of hits to be used in the MIP tracking algorithm. All
  // hits inside the electron ROC (or all hits in the ECal if the event is
  // missing an electron) will be included.
  std::vector<HitData> trackingHitList;

  for (const ldmx::EcalHit &hit : ecalRecHits) {
    // Layer-wise quantities
    ldmx::EcalID id(hit.getID());
    ecalLayerEdepRaw_[id.layer()] =
        ecalLayerEdepRaw_[id.layer()] + hit.getEnergy();
    if (id.layer() >= 20) ecalBackEnergy_ += hit.getEnergy();
    if (maxCellDep_ < hit.getEnergy()) maxCellDep_ = hit.getEnergy();
    if (hit.getEnergy() > 0) {
      nReadoutHits_++;
      ecalLayerEdepReadout_[id.layer()] += hit.getEnergy();
      ecalLayerTime_[id.layer()] += (hit.getEnergy()) * hit.getTime();
      auto [x, y, z] = geometry_->getPosition(id);
      xMean += x * hit.getEnergy();
      yMean += y * hit.getEnergy();
      avgLayerHit_ += id.layer();
      wavgLayerHit += id.layer() * hit.getEnergy();
      if (deepestLayerHit_ < id.layer()) {
        deepestLayerHit_ = id.layer();
      }
      XYCoords xy_pair = std::make_pair(x, y);
      float distance_ele_trajectory =
          ele_trajectory.size()
              ? sqrt(
                    pow((xy_pair.first - ele_trajectory[id.layer()].first), 2) +
                    pow((xy_pair.second - ele_trajectory[id.layer()].second),
                        2))
              : -1.0;
      float distance_photon_trajectory =
          photon_trajectory.size()
              ? sqrt(
                    pow((xy_pair.first - photon_trajectory[id.layer()].first),
                        2) +
                    pow((xy_pair.second - photon_trajectory[id.layer()].second),
                        2))
              : -1.0;

      // Decide which longitudinal segment the hit is in and add to sums
      for (unsigned int iseg = 0; iseg < nsegments; iseg++) {
        if (id.layer() >= segLayers[iseg] &&
            id.layer() <= segLayers[iseg + 1] - 1) {
          energySeg[iseg] += hit.getEnergy();
          xMeanSeg[iseg] += xy_pair.first * hit.getEnergy();
          yMeanSeg[iseg] += xy_pair.second * hit.getEnergy();
          layerMeanSeg[iseg] += id.layer() * hit.getEnergy();

          // Decide which containment region the hit is in and add to sums
          for (unsigned int ireg = 0; ireg < nregions; ireg++) {
            if (distance_ele_trajectory >= ireg * ele_radii[id.layer()] &&
                distance_ele_trajectory < (ireg + 1) * ele_radii[id.layer()]) {
              eContEnergy[ireg][iseg] += hit.getEnergy();
              eContXMean[ireg][iseg] += xy_pair.first * hit.getEnergy();
              eContYMean[ireg][iseg] += xy_pair.second * hit.getEnergy();
            }
            if (distance_photon_trajectory >= ireg * photon_radii[id.layer()] &&
                distance_photon_trajectory <
                    (ireg + 1) * photon_radii[id.layer()]) {
              gContEnergy[ireg][iseg] += hit.getEnergy();
              gContNHits[ireg][iseg] += 1;
              gContXMean[ireg][iseg] += xy_pair.first * hit.getEnergy();
              gContYMean[ireg][iseg] += xy_pair.second * hit.getEnergy();
            }
            if (distance_ele_trajectory > (ireg + 1) * ele_radii[id.layer()] &&
                distance_photon_trajectory >
                    (ireg + 1) * photon_radii[id.layer()]) {
              oContEnergy[ireg][iseg] += hit.getEnergy();
              oContNHits[ireg][iseg] += 1;
              oContXMean[ireg][iseg] += xy_pair.first * hit.getEnergy();
              oContYMean[ireg][iseg] += xy_pair.second * hit.getEnergy();
              oContLayerMean[ireg][iseg] += id.layer() * hit.getEnergy();
            }
          }
        }
      }

      // Decide which containment region the hit is in and add to sums
      for (unsigned int ireg = 0; ireg < nregions; ireg++) {
        if (distance_ele_trajectory >= ireg * ele_radii[id.layer()] &&
            distance_ele_trajectory < (ireg + 1) * ele_radii[id.layer()])
          electronContainmentEnergy[ireg] += hit.getEnergy();
        if (distance_photon_trajectory >= ireg * photon_radii[id.layer()] &&
            distance_photon_trajectory < (ireg + 1) * photon_radii[id.layer()])
          photonContainmentEnergy[ireg] += hit.getEnergy();
        if (distance_ele_trajectory > (ireg + 1) * ele_radii[id.layer()] &&
            distance_photon_trajectory >
                (ireg + 1) * photon_radii[id.layer()]) {
          outsideContainmentEnergy[ireg] += hit.getEnergy();
          outsideContainmentNHits[ireg] += 1;
          outsideContainmentXmean[ireg] += xy_pair.first * hit.getEnergy();
          outsideContainmentYmean[ireg] += xy_pair.second * hit.getEnergy();
        }
      }

      // MIP tracking:  Decide whether hit should be added to trackingHitList
      // If inside e- RoC or if etraj is missing, use the hit for tracking:
      if (distance_ele_trajectory >= ele_radii[id.layer()] ||
          distance_ele_trajectory == -1.0) {
        HitData hd;
        hd.pos = TVector3(xy_pair.first, xy_pair.second,
                          geometry_->getZPosition(id.layer()));
        hd.layer = id.layer();
        trackingHitList.push_back(hd);
      }
    }
  }

  for (const auto &[id, energy] : cellMapTightIso_) {
    if (energy > 0) summedTightIso_ += energy;
  }

  for (int iLayer = 0; iLayer < ecalLayerEdepReadout_.size(); iLayer++) {
    ecalLayerTime_[iLayer] =
        ecalLayerTime_[iLayer] / ecalLayerEdepReadout_[iLayer];
    summedDet_ += ecalLayerEdepReadout_[iLayer];
  }

  if (nReadoutHits_ > 0) {
    avgLayerHit_ /= nReadoutHits_;
    wavgLayerHit /= summedDet_;
    xMean /= summedDet_;
    yMean /= summedDet_;
  } else {
    wavgLayerHit = 0;
    avgLayerHit_ = 0;
    xMean = 0;
    yMean = 0;
  }

  // If necessary, quotient out the total energy from the means
  for (unsigned int iseg = 0; iseg < nsegments; iseg++) {
    if (energySeg[iseg] > 0) {
      xMeanSeg[iseg] /= energySeg[iseg];
      yMeanSeg[iseg] /= energySeg[iseg];
      layerMeanSeg[iseg] /= energySeg[iseg];
    }
    for (unsigned int ireg = 0; ireg < nregions; ireg++) {
      if (eContEnergy[ireg][iseg] > 0) {
        eContXMean[ireg][iseg] /= eContEnergy[ireg][iseg];
        eContYMean[ireg][iseg] /= eContEnergy[ireg][iseg];
      }
      if (gContEnergy[ireg][iseg] > 0) {
        gContXMean[ireg][iseg] /= gContEnergy[ireg][iseg];
        gContYMean[ireg][iseg] /= gContEnergy[ireg][iseg];
      }
      if (oContEnergy[ireg][iseg] > 0) {
        oContXMean[ireg][iseg] /= oContEnergy[ireg][iseg];
        oContYMean[ireg][iseg] /= oContEnergy[ireg][iseg];
        oContLayerMean[ireg][iseg] /= oContEnergy[ireg][iseg];
      }
    }
  }

  for (unsigned int ireg = 0; ireg < nregions; ireg++) {
    if (outsideContainmentEnergy[ireg] > 0) {
      outsideContainmentXmean[ireg] /= outsideContainmentEnergy[ireg];
      outsideContainmentYmean[ireg] /= outsideContainmentEnergy[ireg];
    }
  }

  // Loop over hits a second time to find the standard deviations.
  for (const ldmx::EcalHit &hit : ecalRecHits) {
    ldmx::EcalID id(hit.getID());
    auto [x, y, z] = geometry_->getPosition(id);
    if (hit.getEnergy() > 0) {
      xStd_ += pow((x - xMean), 2) * hit.getEnergy();
      yStd_ += pow((y - yMean), 2) * hit.getEnergy();
      stdLayerHit_ += pow((id.layer() - wavgLayerHit), 2) * hit.getEnergy();
    }
    XYCoords xy_pair = std::make_pair(x, y);
    float distance_ele_trajectory =
        ele_trajectory.size()
            ? sqrt(pow((xy_pair.first - ele_trajectory[id.layer()].first), 2) +
                   pow((xy_pair.second - ele_trajectory[id.layer()].second), 2))
            : -1.0;
    float distance_photon_trajectory =
        photon_trajectory.size()
            ? sqrt(pow((xy_pair.first - photon_trajectory[id.layer()].first),
                       2) +
                   pow((xy_pair.second - photon_trajectory[id.layer()].second),
                       2))
            : -1.0;

    for (unsigned int iseg = 0; iseg < nsegments; iseg++) {
      if (id.layer() >= segLayers[iseg] &&
          id.layer() <= segLayers[iseg + 1] - 1) {
        xStdSeg[iseg] +=
            pow((xy_pair.first - xMeanSeg[iseg]), 2) * hit.getEnergy();
        yStdSeg[iseg] +=
            pow((xy_pair.second - yMeanSeg[iseg]), 2) * hit.getEnergy();
        layerStdSeg[iseg] +=
            pow((id.layer() - layerMeanSeg[iseg]), 2) * hit.getEnergy();

        for (unsigned int ireg = 0; ireg < nregions; ireg++) {
          if (distance_ele_trajectory > (ireg + 1) * ele_radii[id.layer()] &&
              distance_photon_trajectory >
                  (ireg + 1) * photon_radii[id.layer()]) {
            oContXStd[ireg][iseg] +=
                pow((xy_pair.first - oContXMean[ireg][iseg]), 2) *
                hit.getEnergy();
            oContYStd[ireg][iseg] +=
                pow((xy_pair.second - oContYMean[ireg][iseg]), 2) *
                hit.getEnergy();
            oContLayerStd[ireg][iseg] +=
                pow((id.layer() - oContLayerMean[ireg][iseg]), 2) *
                hit.getEnergy();
          }
        }
      }
    }

    for (unsigned int ireg = 0; ireg < nregions; ireg++) {
      if (distance_ele_trajectory > (ireg + 1) * ele_radii[id.layer()] &&
          distance_photon_trajectory > (ireg + 1) * photon_radii[id.layer()]) {
        outsideContainmentXstd[ireg] +=
            pow((xy_pair.first - outsideContainmentXmean[ireg]), 2) *
            hit.getEnergy();
        outsideContainmentYstd[ireg] +=
            pow((xy_pair.second - outsideContainmentYmean[ireg]), 2) *
            hit.getEnergy();
      }
    }
  }

  if (nReadoutHits_ > 0) {
    xStd_ = sqrt(xStd_ / summedDet_);
    yStd_ = sqrt(yStd_ / summedDet_);
    stdLayerHit_ = sqrt(stdLayerHit_ / summedDet_);
  } else {
    xStd_ = 0;
    yStd_ = 0;
    stdLayerHit_ = 0;
  }

  // Quotient out the total energies from the standard deviations if possible
  // and take root
  for (unsigned int iseg = 0; iseg < nsegments; iseg++) {
    if (energySeg[iseg] > 0) {
      xStdSeg[iseg] = sqrt(xStdSeg[iseg] / energySeg[iseg]);
      yStdSeg[iseg] = sqrt(yStdSeg[iseg] / energySeg[iseg]);
      layerStdSeg[iseg] = sqrt(layerStdSeg[iseg] / energySeg[iseg]);
    }
    for (unsigned int ireg = 0; ireg < nregions; ireg++) {
      if (oContEnergy[ireg][iseg] > 0) {
        oContXStd[ireg][iseg] =
            sqrt(oContXStd[ireg][iseg] / oContEnergy[ireg][iseg]);
        oContYStd[ireg][iseg] =
            sqrt(oContYStd[ireg][iseg] / oContEnergy[ireg][iseg]);
        oContLayerStd[ireg][iseg] =
            sqrt(oContLayerStd[ireg][iseg] / oContEnergy[ireg][iseg]);
      }
    }
  }

  for (unsigned int ireg = 0; ireg < nregions; ireg++) {
    if (outsideContainmentEnergy[ireg] > 0) {
      outsideContainmentXstd[ireg] =
          sqrt(outsideContainmentXstd[ireg] / outsideContainmentEnergy[ireg]);
      outsideContainmentYstd[ireg] =
          sqrt(outsideContainmentYstd[ireg] / outsideContainmentEnergy[ireg]);
    }
  }

  // end loop over sim hits

  /* Code for fiducial region below */

  std::vector<float> faceXY(2);

  if (!recoilP.empty() && recoilP[2] != 0) {
    faceXY[0] = ((223.8 - 220.0) * (recoilP[0] / recoilP[2])) + recoilPos[0];
    faceXY[1] = ((223.8 - 220.0) * (recoilP[1] / recoilP[2])) + recoilPos[1];
  } else {
    faceXY[0] = -9999.0;
    faceXY[1] = -9999.0;
  }

  int inside = 0;
  int up = 0;
  int step = 0;
  int index;
  float cell_radius = 5.0;

  std::vector<float>::iterator it;
  it = std::lower_bound(mapsx.begin(), mapsx.end(), faceXY[0]);

  index = std::distance(mapsx.begin(), it);

  if (index == mapsx.size()) {
    index += -1;
  }

  if (!recoilP.empty() && faceXY[0] != -9999.0) {
    while (true) {
      std::vector<double> dis(2);

      dis[0] = faceXY[0] - mapsx[index + step];
      dis[1] = faceXY[1] - mapsy[index + step];

      float celldis = sqrt(pow(dis[0], 2) + pow(dis[1], 2));

      if (celldis <= cell_radius) {
        inside = 1;
        break;
      }

      if ((abs(dis[0]) > 5 && up == 0) || index + step == mapsx.size() - 1) {
        up = 1;
        step = 0;
      } else if ((abs(dis[0]) > 5 && up == 1) ||
                 (index + step == 0 && up == 1)) {
        break;
      }

      if (up == 0) {
        step += 1;
      } else {
        step += -1;
      }
    }
  }

  // MIP tracking starts here

  /* Goal:  Calculate
   *  nStraightTracks (self-explanatory),
   *  nLinregTracks (tracks found by linreg algorithm),
   */

  // Find epAng and epSep, and prepare EP trajectory vectors:
  TVector3 e_traj_start;
  TVector3 e_traj_end;
  TVector3 p_traj_start;
  TVector3 p_traj_end;
  if (ele_trajectory.size() > 0 && photon_trajectory.size() > 0) {
    // Create TVector3s marking the start and endpoints of each projected
    // trajectory
    e_traj_start.SetXYZ(ele_trajectory[0].first, ele_trajectory[0].second,
                        geometry_->getZPosition(0));
    e_traj_end.SetXYZ(ele_trajectory[(nEcalLayers_ - 1)].first,
                      ele_trajectory[(nEcalLayers_ - 1)].second,
                      geometry_->getZPosition((nEcalLayers_ - 1)));
    p_traj_start.SetXYZ(photon_trajectory[0].first, photon_trajectory[0].second,
                        geometry_->getZPosition(0));
    p_traj_end.SetXYZ(photon_trajectory[(nEcalLayers_ - 1)].first,
                      photon_trajectory[(nEcalLayers_ - 1)].second,
                      geometry_->getZPosition((nEcalLayers_ - 1)));

    TVector3 evec = e_traj_end - e_traj_start;
    TVector3 e_norm = evec.Unit();
    TVector3 pvec = p_traj_end - p_traj_start;
    TVector3 p_norm = pvec.Unit();
    epAng_ = acos(epDot_) * 180.0 / M_PI;
    epSep_ = sqrt(pow(e_traj_start.X() - p_traj_start.X(), 2) +
                  pow(e_traj_start.Y() - p_traj_start.Y(), 2));
  } else {
    // Electron trajectory is missing, so all hits in the Ecal are fair game.
    // Pick e/ptraj so that they won't restrict the tracking algorithm (place
    // them far outside the ECal).
    e_traj_start = TVector3(999, 999, geometry_->getZPosition(0));  // 0);
    e_traj_end = TVector3(
        999, 999, geometry_->getZPosition((nEcalLayers_ - 1)));       // 999);
    p_traj_start = TVector3(1000, 1000, geometry_->getZPosition(0));  // 0);
    p_traj_end = TVector3(
        1000, 1000, geometry_->getZPosition((nEcalLayers_ - 1)));  // 1000);
    /*ensures event will not be vetoed by angle/separation cut */
    epAng_ = 3.0 + 1.0;
    epSep_ = 10.0 + 1.0;
    epDot_ = 1.0;  // default to 1.0 (?)
  }

  // Near photon step:  Find the first layer of the ECal where a hit near the
  // projected photon trajectory is found Currently unusued pending further
  // study; performance has dropped between v9 and v12. Currently used in
  // segmipBDT
  firstNearPhLayer_ = nEcalLayers_ - 1;

  if (photon_trajectory.size() !=
      0) {  // If no photon trajectory, leave this at the default (ECal back)
    for (std::vector<HitData>::iterator it = trackingHitList.begin();
         it != trackingHitList.end(); ++it) {
      float ehDist =
          sqrt(pow((*it).pos.X() - photon_trajectory[(*it).layer].first, 2) +
               pow((*it).pos.Y() - photon_trajectory[(*it).layer].second, 2));
      if (ehDist < 8.7) {
        nNearPhHits_++;
        if ((*it).layer < firstNearPhLayer_) {
          firstNearPhLayer_ = (*it).layer;
        }
      }
    }
  }

  // Territories limited to trackingHitList
  TVector3 gToe = (e_traj_start - p_traj_start).Unit();
  TVector3 origin = p_traj_start + 0.5 * 8.7 * gToe;
  if (ele_trajectory.size() > 0) {
    for (auto &hitData : trackingHitList) {
      TVector3 hitPos = hitData.pos;
      TVector3 hitPrime = hitPos - origin;
      if (hitPrime.Dot(gToe) <= 0) {
        photonTerritoryHits_++;
      }
    }
  } else {
    photonTerritoryHits_ = nReadoutHits_;
  }

  // Find straight MIP tracks:

  std::sort(trackingHitList.begin(), trackingHitList.end(),
            [](HitData ha, HitData hb) { return ha.layer > hb.layer; });
  // For merging tracks:  Need to keep track of existing tracks
  // Candidate tracks to merge in will always be in front of the current track
  // (lower z), so only store the last hit 3-layer vector:  each track = vector
  // of 3-tuples (xy+layer).
  std::vector<std::vector<HitData>> track_list;

  // print trackingHitList
  if (verbose_) {
    ldmx_log(debug) << "====== Tracking hit list (original) length"
                    << trackingHitList.size() << " ======";
    for (int i = 0; i < trackingHitList.size(); i++) {
      std::cout << "[" << trackingHitList[i].pos.X() << ", "
                << trackingHitList[i].pos.Y() << ", "
                << trackingHitList[i].layer << "], ";
    }
    std::cout << std::endl;
    ldmx_log(debug) << "====== END OF Tracking hit list ======";
  }

  float cellWidth = 8.7;
  for (int iHit = 0; iHit < trackingHitList.size(); iHit++) {
    int track[34];  // list of hit numbers in track (34 = maximum theoretical
                    // length)
    int currenthit;
    int trackLen;

    track[0] = iHit;
    currenthit = iHit;
    trackLen = 1;

    // Search for hits to add to the track:
    // repeatedly find hits in the front two layers with same x & y positions
    // but since v14 the odd layers are offset, so we allow half a cellWidth
    // deviation and then add to track until no more hits are found
    int jHit = iHit;
    while (jHit < trackingHitList.size()) {
      if ((trackingHitList[jHit].layer ==
               trackingHitList[currenthit].layer - 1 ||
           trackingHitList[jHit].layer ==
               trackingHitList[currenthit].layer - 2) &&
          abs(trackingHitList[jHit].pos.X() -
              trackingHitList[currenthit].pos.X()) <= 0.5 * cellWidth &&
          abs(trackingHitList[jHit].pos.Y() -
              trackingHitList[currenthit].pos.Y()) <= 0.5 * cellWidth) {
        track[trackLen] = jHit;
        trackLen++;
        currenthit = jHit;
      }
      jHit++;
    }

    // Confirm that the track is valid:
    if (trackLen < 2) continue;  // Track must contain at least 2 hits
    float closest_e = distTwoLines(trackingHitList[track[0]].pos,
                                   trackingHitList[track[trackLen - 1]].pos,
                                   e_traj_start, e_traj_end);
    float closest_p = distTwoLines(trackingHitList[track[0]].pos,
                                   trackingHitList[track[trackLen - 1]].pos,
                                   p_traj_start, p_traj_end);
    // Make sure that the track is near the photon trajectory and away from the
    // electron trajectory Details of these constraints may be revised
    if (closest_p > cellWidth and closest_e < 2 * cellWidth) continue;
    if (trackLen < 4 and closest_e > closest_p) continue;
    if (verbose_) {
      ldmx_log(debug) << "====== After rejection for MIP tracking ======";
      ldmx_log(debug) << "current hit: [" << trackingHitList[iHit].pos.X()
                      << ", " << trackingHitList[iHit].pos.Y() << ", "
                      << trackingHitList[iHit].layer << "]";

      for (int k = 0; k < trackLen; k++) {
        ldmx_log(debug) << "track[" << k << "] position = ["
                        << trackingHitList[track[k]].pos.X() << ", "
                        << trackingHitList[track[k]].pos.Y() << ", "
                        << trackingHitList[track[k]].layer << "]";
      }
    }

    // if track found, increment nStraightTracks and remove all hits in track
    // from future consideration
    if (trackLen >= 2) {
      std::vector<HitData> temp_track_list;
      int n_remove = 0;
      for (int kHit = 0; kHit < trackLen; kHit++) {
        temp_track_list.push_back(trackingHitList[track[kHit] - n_remove]);
        trackingHitList.erase(trackingHitList.begin() + track[kHit] - n_remove);
        n_remove++;
      }
      // print trackingHitList
      if (verbose_) {
        ldmx_log(debug) << "====== Tracking hit list (after erase) length"
                        << trackingHitList.size() << " ======";
        for (int i = 0; i < trackingHitList.size(); i++) {
          std::cout << "[" << trackingHitList[i].pos.X() << ", "
                    << trackingHitList[i].pos.Y() << ", "
                    << trackingHitList[i].layer << "] ";
        }
        std::cout << std::endl;
        ldmx_log(debug) << "====== END OF Tracking hit list ======";
      }

      track_list.push_back(temp_track_list);
      // The *current* hit will have been removed, so iHit is currently pointing
      // to the next hit. Decrement iHit so no hits will get skipped by iHit++
      iHit--;
    }
  }

  ldmx_log(debug) << "Straight tracks found (before merge): "
                  << track_list.size();
  if (verbose_) {
    for (int iTrack = 0; iTrack < track_list.size(); iTrack++) {
      ldmx_log(debug) << "Track " << iTrack << ":";
      for (int iHit = 0; iHit < track_list[iTrack].size(); iHit++) {
        std::cout << "  Hit " << iHit << ": ["
                  << track_list[iTrack][iHit].pos.X() << ", "
                  << track_list[iTrack][iHit].pos.Y() << ", "
                  << track_list[iTrack][iHit].layer << "]" << std::endl;
      }
      std::cout << std::endl;
    }
  }

  // Optional addition:  Merge nearby straight tracks.  Not necessary for veto.
  // Criteria:  consider tail of track.  Merge if head of next track is 1/2
  // layers behind, within 1 cell of xy position.
  ldmx_log(debug) << "Beginning track merging using " << track_list.size()
                  << " tracks";

  for (int track_i = 0; track_i < track_list.size(); track_i++) {
    // for each track, check the remainder of the track list for compatible
    // tracks
    std::vector<HitData> base_track = track_list[track_i];
    HitData tail_hitdata = base_track.back();  // xylayer of last hit in track
    if (verbose_) ldmx_log(debug) << "  Considering track " << track_i;
    for (int track_j = track_i + 1; track_j < track_list.size(); track_j++) {
      if (verbose_)
        ldmx_log(debug) << "    Checking for compatibility: " << track_j;
      std::vector<HitData> checking_track = track_list[track_j];
      HitData head_hitdata = checking_track.front();
      // if 1-2 layers behind, and xy within one cell...
      if ((head_hitdata.layer == tail_hitdata.layer + 1 ||
           head_hitdata.layer == tail_hitdata.layer + 2) &&
          pow(pow(head_hitdata.pos.X() - tail_hitdata.pos.X(), 2) +
                  pow(head_hitdata.pos.Y() - tail_hitdata.pos.Y(), 2),
              0.5) <= cellWidth) {
        // ...then append the second track to the first one and delete it
        // NOTE:  TO ADD:  (trackingHitList[iHit].pos -
        // trackingHitList[jHit].pos).Mag()
        if (verbose_) {
          ldmx_log(debug) << "     **Compatible track found!  Adding track, "
                             "deleting stuff...";
          ldmx_log(debug) << "     Tail xylayer: " << head_hitdata.pos.X()
                          << "," << head_hitdata.pos.Y() << ","
                          << head_hitdata.layer;
          ldmx_log(debug) << "     Head xylayer: " << tail_hitdata.pos.X()
                          << "," << tail_hitdata.pos.Y() << ","
                          << tail_hitdata.layer;
        }
        for (int hit_k = 0; hit_k < checking_track.size(); hit_k++) {
          base_track.push_back(track_list[track_j][hit_k]);
        }
        track_list[track_i] = base_track;
        track_list.erase(track_list.begin() + track_j);
        break;
      }
    }
  }
  nStraightTracks_ = track_list.size();
  // print the track list
  ldmx_log(debug) << "Straight tracks found (after merge): "
                  << nStraightTracks_;
  for (int track_i = 0; track_i < track_list.size(); track_i++) {
    ldmx_log(debug) << "Track " << track_i << ":";
    for (int hit_i = 0; hit_i < track_list[track_i].size(); hit_i++) {
      ldmx_log(debug) << "  Hit " << hit_i << ": ["
                      << track_list[track_i][hit_i].pos.X() << ", "
                      << track_list[track_i][hit_i].pos.Y() << ", "
                      << track_list[track_i][hit_i].layer << "]";
    }
  }

  // Linreg tracking:

  ldmx_log(debug) << "Finding linreg tracks from " << trackingHitList.size()
                  << " hits";

  for (int iHit = 0; iHit < trackingHitList.size(); iHit++) {
    int track[34];
    int trackLen;
    int currenthit;
    int hitsInRegion[50];  // Hits being considered at one time
    int nHitsInRegion;     // Number of hits under consideration
    TMatrixD svdMatrix(3, 3);
    TMatrixD Vm(3, 3);
    TMatrixD hdt(3, 3);
    TVector3 slopeVec;
    TVector3 hmean;
    TVector3 hpoint;
    float r_corr_best;
    int hitNums_best[3];  // Hit numbers of current best track candidate
    int hitNums[3];

    trackLen = 0;
    nHitsInRegion = 1;
    currenthit = iHit;
    hitsInRegion[0] = iHit;
    // Find all hits within 2 cells of the primary hit:
    for (int jHit = 0; jHit < trackingHitList.size(); jHit++) {
      float dstToHit =
          (trackingHitList[iHit].pos - trackingHitList[jHit].pos).Mag();
      if (dstToHit <= 2 * cellWidth) {
        hitsInRegion[nHitsInRegion] = jHit;
        nHitsInRegion++;
      }
    }

    // Look at combinations of hits within the region (do not consider the same
    // combination twice):
    hitNums[0] = iHit;
    for (int jHit = 1; jHit < nHitsInRegion - 1; jHit++) {
      if (trackingHitList.size() < 3) break;
      hitNums[1] = jHit;
      for (int kHit = jHit + 1; kHit < nHitsInRegion; kHit++) {
        hitNums[2] = kHit;
        for (int hInd = 0; hInd < 3; hInd++) {
          // hmean = geometric mean, subtract off from hits to improve SVD
          // performance
          hmean(hInd) = (trackingHitList[hitNums[0]].pos(hInd) +
                         trackingHitList[hitNums[1]].pos(hInd) +
                         trackingHitList[hitNums[2]].pos(hInd)) /
                        3.0;
        }
        for (int hInd = 0; hInd < 3; hInd++) {
          for (int lInd = 0; lInd < 3; lInd++) {
            hdt(hInd, lInd) =
                trackingHitList[hitNums[hInd]].pos(lInd) - hmean(lInd);
          }
        }

        // Perform "linreg" on selected points:
        TDecompSVD svdObj = TDecompSVD(hdt);
        bool decomposed = svdObj.Decompose();
        if (!decomposed) continue;

        Vm = svdObj.GetV();  // First col of V matrix is the slope of the
                             // best-fit line
        for (int hInd = 0; hInd < 3; hInd++) {
          slopeVec(hInd) = Vm[0][hInd];
        }
        // hmean, hpoint are points on the best-fit line
        hpoint = slopeVec + hmean;
        // linreg complete:  Now have best-fit line for 3 hits under
        // consideration Check whether the track is valid:  r^2 must be high,
        // and the track must plausibly originate from the photon
        float closest_e = distTwoLines(hmean, hpoint, e_traj_start, e_traj_end);
        float closest_p = distTwoLines(hmean, hpoint, p_traj_start, p_traj_end);
        // Projected track must be close to the photon; details may change after
        // future study.
        if (closest_p > cellWidth or closest_e < 1.5 * cellWidth) continue;
        // find r^2
        // ~variance
        float vrnc = (trackingHitList[hitNums[0]].pos - hmean).Mag() +
                     (trackingHitList[hitNums[1]].pos - hmean).Mag() +
                     (trackingHitList[hitNums[2]].pos - hmean).Mag();
        // sum of |errors|
        float sumerr =
            distPtToLine(trackingHitList[hitNums[0]].pos, hmean, hpoint) +
            distPtToLine(trackingHitList[hitNums[1]].pos, hmean, hpoint) +
            distPtToLine(trackingHitList[hitNums[2]].pos, hmean, hpoint);
        float r_corr = 1 - sumerr / vrnc;
        // Check whether r^2 exceeds a low minimum r_corr:  "Fake" tracks are
        // still much more common in background, so making the algorithm
        // oversensitive doesn't lower performance significantly
        if (r_corr > r_corr_best and r_corr > .6) {
          r_corr_best = r_corr;
          trackLen = 0;
          // Only looking for 3-hit tracks currently
          for (int k = 0; k < 3; k++) {
            track[k] = hitNums[k];
            trackLen++;
          }
        }
      }
    }
    // Ordinarily, additional hits in line w/ track would be added here.
    // However, this doesn't affect the results of the simple veto. Exclude all
    // hits in a found track from further consideration:
    if (trackLen >= 2) {
      nLinregTracks_++;
      for (int kHit = 0; kHit < trackLen; kHit++) {
        trackingHitList.erase(trackingHitList.begin() + track[kHit]);
      }
      iHit--;
    }
  }

  ldmx_log(debug) << "  MIP tracking completed; found " << nStraightTracks_
                  << " straight tracks and " << nLinregTracks_
                  << " lin-reg tracks";

  result.setVariables(
      nReadoutHits_, deepestLayerHit_, summedDet_, summedTightIso_, maxCellDep_,
      showerRMS_, xStd_, yStd_, avgLayerHit_, stdLayerHit_, ecalBackEnergy_,
      nStraightTracks_, nLinregTracks_, firstNearPhLayer_, nNearPhHits_,
      photonTerritoryHits_, epAng_, epSep_, epDot_, electronContainmentEnergy,
      photonContainmentEnergy, outsideContainmentEnergy,
      outsideContainmentNHits, outsideContainmentXstd, outsideContainmentYstd,
      energySeg, xMeanSeg, yMeanSeg, xStdSeg, yStdSeg, layerMeanSeg,
      layerStdSeg, eContEnergy, eContXMean, eContYMean, gContEnergy, gContNHits,
      gContXMean, gContYMean, oContEnergy, oContNHits, oContXMean, oContYMean,
      oContXStd, oContYStd, oContLayerMean, oContLayerStd,
      ecalLayerEdepReadout_, recoilP, recoilPos);

  if (doBdt_) {
    buildBDTFeatureVector(result);
    ldmx::Ort::FloatArrays inputs({bdtFeatures_});
    float pred =
        rt_->run({featureListName_}, inputs, {"probabilities"})[0].at(1);
    // Removing electron-photon separation step, near photon step due to lower
    // v12 performance; may reconsider
    bool passesTrackingVeto = (nStraightTracks_ < 3) && (nLinregTracks_ == 0);
    //&&  //Commenting the remainder for now
    //(firstNearPhLayer_ >= 6); //&& (epAng_ > 3.0 || epSep_ > 10.0);
    result.setVetoResult(pred > bdtCutVal_ && passesTrackingVeto);
    result.setDiscValue(pred);
    ldmx_log(debug) << "  The pred > bdtCutVal = " << (pred > bdtCutVal_);

    // If the event passes the veto, keep it. Otherwise,
    // drop the event.
    if (result.passesVeto() && inside) {
      setStorageHint(framework::hint_shouldKeep);
    } else {
      setStorageHint(framework::hint_shouldDrop);
    }
  }

  if (inside) {
    setStorageHint(framework::hint_shouldKeep);
  } else {
    setStorageHint(framework::hint_shouldDrop);
  }
  event.add(collectionName_, result);
}

/* Function to calculate the energy weighted shower centroid */
ldmx::EcalID EcalVetoProcessor::GetShowerCentroidIDAndRMS(
    const std::vector<ldmx::EcalHit> &ecalRecHits, double &showerRMS) {
  auto wgtCentroidCoords = std::make_pair<float, float>(0., 0.);
  float sumEdep = 0;
  ldmx::EcalID returnCellId;

  // Calculate Energy Weighted Centroid
  for (const ldmx::EcalHit &hit : ecalRecHits) {
    ldmx::EcalID id(hit.getID());
    CellEnergyPair cell_energy_pair = std::make_pair(id, hit.getEnergy());
    auto [x, y, z] = geometry_->getPosition(id);
    XYCoords centroidCoords = std::make_pair(x, y);
    wgtCentroidCoords.first = wgtCentroidCoords.first +
                              centroidCoords.first * cell_energy_pair.second;
    wgtCentroidCoords.second = wgtCentroidCoords.second +
                               centroidCoords.second * cell_energy_pair.second;
    sumEdep += cell_energy_pair.second;
  }
  wgtCentroidCoords.first = (sumEdep > 1E-6) ? wgtCentroidCoords.first / sumEdep
                                             : wgtCentroidCoords.first;
  wgtCentroidCoords.second = (sumEdep > 1E-6)
                                 ? wgtCentroidCoords.second / sumEdep
                                 : wgtCentroidCoords.second;
  // Find Nearest Cell to Centroid
  float maxDist = 1e6;
  for (const ldmx::EcalHit &hit : ecalRecHits) {
    auto [x, y, z] = geometry_->getPosition(hit.getID());
    XYCoords centroidCoords = std::make_pair(x, y);

    float deltaR =
        pow(pow((centroidCoords.first - wgtCentroidCoords.first), 2) +
                pow((centroidCoords.second - wgtCentroidCoords.second), 2),
            .5);
    showerRMS += deltaR * hit.getEnergy();
    if (deltaR < maxDist) {
      maxDist = deltaR;
      returnCellId = ldmx::EcalID(hit.getID());
    }
  }
  if (sumEdep > 0) showerRMS = showerRMS / sumEdep;
  return ldmx::EcalID(0, returnCellId.module(),
                      returnCellId.cell());  // flatten
}

/**
 * Function to load up empty vector of hit maps
 */
void EcalVetoProcessor::fillHitMap(
    const std::vector<ldmx::EcalHit> &ecalRecHits,
    std::map<ldmx::EcalID, float> &cellMap_) {
  for (const ldmx::EcalHit &hit : ecalRecHits) {
    ldmx::EcalID id(hit.getID());
    cellMap_.emplace(id, hit.getEnergy());
  }
}

void EcalVetoProcessor::fillIsolatedHitMap(
    const std::vector<ldmx::EcalHit> &ecalRecHits, ldmx::EcalID globalCentroid,
    std::map<ldmx::EcalID, float> &cellMap_,
    std::map<ldmx::EcalID, float> &cellMapIso_, bool doTight) {
  for (const ldmx::EcalHit &hit : ecalRecHits) {
    auto isolatedHit = std::make_pair(true, ldmx::EcalID());
    ldmx::EcalID id(hit.getID());
    if (doTight) {
      // Disregard hits that are on the centroid.
      if (id == globalCentroid) continue;

      // Skip hits that are on centroid inner ring
      if (geometry_->isNN(globalCentroid, id)) {
        continue;
      }
    }

    // Skip hits that have a readout neighbor
    // Get neighboring cell id's and try to look them up in the full cell map
    // (constant speed algo.)
    //  these ideas are only cell/module (must ignore layer)
    std::vector<ldmx::EcalID> cellNbrIds = geometry_->getNN(id);

    for (int k = 0; k < cellNbrIds.size(); k++) {
      // update neighbor ID to the current layer
      cellNbrIds[k] = ldmx::EcalID(id.layer(), cellNbrIds[k].module(),
                                   cellNbrIds[k].cell());
      // look in cell hit map to see if it is there
      if (cellMap_.find(cellNbrIds[k]) != cellMap_.end()) {
        isolatedHit = std::make_pair(false, cellNbrIds[k]);
        break;
      }
    }
    if (!isolatedHit.first) {
      continue;
    }
    // Insert isolated hit
    cellMapIso_.emplace(id, hit.getEnergy());
  }
}

/* Calculate where trajectory intersects ECAL layers using position and momentum
 * at scoring plane */
std::vector<std::pair<float, float>> EcalVetoProcessor::getTrajectory(
    std::vector<double> momentum, std::vector<float> position) {
  std::vector<XYCoords> positions;
  for (int iLayer = 0; iLayer < nEcalLayers_; iLayer++) {
    float posX =
        position[0] + (momentum[0] / momentum[2]) *
                          (geometry_->getZPosition(iLayer) - position[2]);
    float posY =
        position[1] + (momentum[1] / momentum[2]) *
                          (geometry_->getZPosition(iLayer) - position[2]);
    positions.push_back(std::make_pair(posX, posY));
  }
  return positions;
}

// MIP tracking functions:

float EcalVetoProcessor::distTwoLines(TVector3 v1, TVector3 v2, TVector3 w1,
                                      TVector3 w2) {
  TVector3 e1 = v1 - v2;
  TVector3 e2 = w1 - w2;
  TVector3 crs = e1.Cross(e2);
  if (crs.Mag() == 0) {
    return 100.0;  // arbitrary large number; edge case that shouldn't cause
                   // problems.
  } else {
    return std::abs(crs.Dot(v1 - w1) / crs.Mag());
  }
}

float EcalVetoProcessor::distPtToLine(TVector3 h1, TVector3 p1, TVector3 p2) {
  return ((h1 - p1).Cross(h1 - p2)).Mag() / (p1 - p2).Mag();
}

}  // namespace ecal

DECLARE_PRODUCER_NS(ecal, EcalVetoProcessor);
