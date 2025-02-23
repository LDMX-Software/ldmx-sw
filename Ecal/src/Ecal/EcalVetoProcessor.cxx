#include "Ecal/EcalVetoProcessor.h"

// LDMX
#include "DetDescr/EcalGeometry.h"
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
#include <iomanip>

// ROOT (MIP tracking)
#include "TDecompSVD.h"
#include "TMatrixD.h"
#include "TVector3.h"

namespace ecal {

void EcalVetoProcessor::onNewRun(const ldmx::RunHeader &rh) {
  profiling_map_["setup"] = 0.;
  profiling_map_["recoil_electron"] = 0.;
  profiling_map_["trajectories"] = 0.;
  profiling_map_["roc_var"] = 0.;
  profiling_map_["fill_hitmaps"] = 0.;
  profiling_map_["containment_var"] = 0.;
  profiling_map_["mip_tracking_setup"] = 0.;
  profiling_map_["set_variables"] = 0.;
  profiling_map_["bdt_variables"] = 0.;
}

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
  bdtFeatures_.push_back(-1.);
  // bdtFeatures_.push_back(result.getNStraightTracks());
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
  featureListName_ = parameters.getParameter<std::string>("feature_list_name");

  sim_particles_passname_ =
      parameters.getParameter<std::string>("sim_particles_passname");
  // Load BDT ONNX file
  rt_ = std::make_unique<ldmx::Ort::ONNXRuntime>(
      parameters.getParameter<std::string>("bdt_file"));

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
    std::vector<float> values;

    // Read data, line by line
    while (std::getline(rocfile, line)) {
      std::stringstream ss(line);
      values.clear();
      while (std::getline(ss, value, ',')) {
        float f_value = (value != "") ? std::stof(value) : -1.0;
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
  run_lin_reg_ = parameters.getParameter<bool>("run_lin_reg");
  linreg_radius_ = parameters.getParameter<double>("linreg_radius");

  // Set the collection name as defined in the configuration
  sp_pass_name_ = parameters.getParameter<std::string>("sp_pass_name");
  collectionName_ = parameters.getParameter<std::string>("collection_name");
  rec_pass_name_ = parameters.getParameter<std::string>("rec_pass_name");
  rec_coll_name_ = parameters.getParameter<std::string>("rec_coll_name");
  recoil_from_tracking_ = parameters.getParameter<bool>("recoil_from_tracking");
  track_collection_ = parameters.getParameter<std::string>("track_collection");
  track_pass_name_ =
      parameters.getParameter<std::string>("track_pass_name", "");
  inverse_skim_ = parameters.getParameter<bool>("inverse_skim");
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
  epAngAtTarget_ = 0;
  epSep_ = 0;
  epDot_ = 0;
  epDotAtTarget_ = 0;
  photonTerritoryHits_ = 0;

  std::fill(ecalLayerEdepRaw_.begin(), ecalLayerEdepRaw_.end(), 0);
  std::fill(ecalLayerEdepReadout_.begin(), ecalLayerEdepReadout_.end(), 0);
  std::fill(ecalLayerTime_.begin(), ecalLayerTime_.end(), 0);
}

void EcalVetoProcessor::produce(framework::Event &event) {
  auto start = std::chrono::high_resolution_clock::now();
  nevents_++;

  // Get the Ecal Geometry
  geometry_ = &getCondition<ldmx::EcalGeometry>(
      ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  ldmx::EcalVetoResult result;

  clearProcessor();

  // Get the collection of Ecal scoring plane hits. If it doesn't exist,
  // don't bother adding any truth tracking information.

  std::array<float, 3> recoilP = {0., 0., 0.};
  std::array<float, 3> recoilPos = {-9999., -9999., -9999.};
  std::array<float, 3> recoilPAtTarget = {0., 0., 0.};
  std::array<float, 3> recoilPosAtTarget = {-9999., -9999., -9999.};

  auto setup = std::chrono::high_resolution_clock::now();
  profiling_map_["setup"] +=
      std::chrono::duration<float, std::milli>(setup - start).count();

  if (!recoil_from_tracking_ &&
      event.exists("EcalScoringPlaneHits", sp_pass_name_)) {
    ldmx_log(trace) << "   Loop through all of the sim particles and find the "
                       "recoil electron";
    //
    // Loop through all of the sim particles and find the recoil electron.
    //

    // Get the collection of simulated particles from the event
    auto particleMap{event.getMap<int, ldmx::SimParticle>(
        "SimParticles", sim_particles_passname_)};

    // Search for the recoil electron
    auto [recoilTrackID, recoilElectron] = Analysis::getRecoil(particleMap);

    // Find ECAL SP hit for recoil electron
    auto ecalSpHits{event.getCollection<ldmx::SimTrackerHit>(
        "EcalScoringPlaneHits", sp_pass_name_)};
    float pmax = 0;
    for (ldmx::SimTrackerHit &spHit : ecalSpHits) {
      ldmx::SimSpecialID hit_id(spHit.getID());
      auto ecal_sp_momentum = spHit.getMomentum();
      auto ecal_sp_position = spHit.getPosition();
      if (hit_id.plane() != 31 || ecal_sp_momentum[2] <= 0) continue;

      if (spHit.getTrackID() == recoilTrackID) {
        // A*A is faster than pow(A,2)
        if (sqrt((ecal_sp_momentum[0] * ecal_sp_momentum[0]) +
                 (ecal_sp_momentum[1] * ecal_sp_momentum[1]) +
                 (ecal_sp_momentum[2] * ecal_sp_momentum[2])) > pmax) {
          recoilP = {static_cast<float>(ecal_sp_momentum[0]),
                     static_cast<float>(ecal_sp_momentum[1]),
                     static_cast<float>(ecal_sp_momentum[2])};
          recoilPos = {(ecal_sp_position[0]), (ecal_sp_position[1]),
                       (ecal_sp_position[2])};
          pmax = sqrt(recoilP[0] * recoilP[0] + recoilP[1] * recoilP[1] +
                      recoilP[2] * recoilP[2]);
        }
      }
    }

    // Find target SP hit for recoil electron
    if (event.exists("TargetScoringPlaneHits", sp_pass_name_)) {
      std::vector<ldmx::SimTrackerHit> targetSpHits =
          event.getCollection<ldmx::SimTrackerHit>("TargetScoringPlaneHits",
                                                   sp_pass_name_);
      pmax = 0;
      for (ldmx::SimTrackerHit &spHit : targetSpHits) {
        ldmx::SimSpecialID hit_id(spHit.getID());
        auto target_sp_momentum = spHit.getMomentum();
        auto target_sp_position = spHit.getPosition();
        if (hit_id.plane() != 1 || target_sp_momentum[2] <= 0) continue;

        if (spHit.getTrackID() == recoilTrackID) {
          if (sqrt((target_sp_momentum[0] * target_sp_momentum[0]) +
                   (target_sp_momentum[1] * target_sp_momentum[1]) +
                   (target_sp_momentum[2] * target_sp_momentum[2])) > pmax) {
            recoilPAtTarget = {static_cast<float>(target_sp_momentum[0]),
                               static_cast<float>(target_sp_momentum[1]),
                               static_cast<float>(target_sp_momentum[2])};
            recoilPosAtTarget = {target_sp_position[0], target_sp_position[1],
                                 target_sp_position[2]};
            // (A*A) is faster than pow(A,2)
            pmax = sqrt((recoilPAtTarget[0] * recoilPAtTarget[0]) +
                        (recoilPAtTarget[1] * recoilPAtTarget[1]) +
                        (recoilPAtTarget[2] * recoilPAtTarget[2]));
          }
        }
      }  // end loop on target SP hits
    }  // end condition on target SP
  }  // end condition on ecal SP

  // Get recoilPos using recoil tracking
  bool fiducial_in_tracker{false};
  if (recoil_from_tracking_) {
    ldmx_log(trace) << "  Get recoil tracks collection";

    // Get the recoil track collection
    auto recoil_tracks{
        event.getCollection<ldmx::Track>(track_collection_, track_pass_name_)};

    ldmx_log(trace) << "  Propagate the recoil ele to the ECAL";
    ldmx::TrackStateType ts_type = ldmx::TrackStateType::AtECAL;
    auto recoil_track_states_ecal = trackProp(recoil_tracks, ts_type, "ecal");
    ldmx_log(trace) << "  Propagate the recoil ele to the Target";
    ldmx::TrackStateType ts_type_target = ldmx::TrackStateType::AtTarget;
    auto recoil_track_states_target =
        trackProp(recoil_tracks, ts_type_target, "target");

    ldmx_log(trace) << "  Set recoilPos and recoilP";
    // Redefining recoilPos now to come from the track state
    // track_state_loc0 is recoilPos[0] and track_state_loc1 is recoilPos[1]
    if (!recoil_track_states_ecal.empty()) {
      fiducial_in_tracker = true;
      recoilPos = {recoil_track_states_ecal[0], recoil_track_states_ecal[1],
                   recoil_track_states_ecal[2]};
      recoilP = {(recoil_track_states_ecal[3]), (recoil_track_states_ecal[4]),
                 (recoil_track_states_ecal[5])};
    } else {
      ldmx_log(trace) << "  No recoil track at ECAL";
      fiducial_in_tracker = false;
    }
    ldmx_log(debug) << "  Set recoilP = (" << recoilP[0] << ", " << recoilP[1]
                    << ", " << recoilP[2] << ") and recoilPos = ("
                    << recoilPos[0] << ", " << recoilPos[1] << ", "
                    << recoilPos[2] << ")";

    // Repeat the above but now for the taget states
    if (!recoil_track_states_target.empty()) {
      recoilPosAtTarget = {(recoil_track_states_target[0]),
                           (recoil_track_states_target[1]),
                           (recoil_track_states_target[2])};
      recoilPAtTarget = {recoil_track_states_target[3],
                         recoil_track_states_target[4],
                         recoil_track_states_target[5]};
    }
    ldmx_log(debug) << "  Set recoilPAtTarget = (" << recoilPAtTarget[0] << ", "
                    << recoilPAtTarget[1] << ", " << recoilPAtTarget[2]
                    << ") and recoilPosAtTarget = (" << recoilPosAtTarget[0]
                    << ", " << recoilPosAtTarget[1] << ", "
                    << recoilPosAtTarget[2] << ")";
  }  // condition to do recoil information from tracking

  ldmx_log(trace) << "   Get projected trajectories for electron and photon";

  auto recoil_electron = std::chrono::high_resolution_clock::now();
  profiling_map_["recoil_electron"] +=
      std::chrono::duration<float, std::milli>(recoil_electron - setup).count();

  // Get projected trajectories for electron and photon
  std::vector<XYCoords> ele_trajectory, photon_trajectory,
      ele_trajectory_at_target;
  // Require that z-momentum is positive (which will also exclude the default
  // initializaton) Require that the positions are not the default initializaton
  if ((recoilP[2] > 0.) && (recoilPAtTarget[2] > 0.) &&
      (recoilPos[0] != -9999.) && (recoilPosAtTarget[0] != -9999.)) {
    ele_trajectory = getTrajectory(recoilP, recoilPos);
    // Get the photon projection. This does not require that the photon exists
    // tho
    std::array<float, 3> photon_proj_momentum = {
        -recoilPAtTarget[0], -recoilPAtTarget[1],
        beamEnergyMeV_ - recoilPAtTarget[2]};
    photon_trajectory = getTrajectory(photon_proj_momentum, recoilPosAtTarget);
  } else {
    ldmx_log(trace) << "Ele / photon trajectory cannot be determined, pZ = "
                    << recoilP[2] << " pZAtTarget = " << recoilPAtTarget[2]
                    << " X = " << recoilPos[0]
                    << " XAtTarget = " << recoilPosAtTarget[0];
  }

  float recoilPMag = (recoilP[2] > 0.) ? sqrt((recoilP[0] * recoilP[0]) +
                                              (recoilP[1] * recoilP[1]) +
                                              (recoilP[2] * recoilP[2]))
                                       : -1.0;
  float recoilTheta =
      recoilPMag > 0 ? acos(recoilP[2] / recoilPMag) * 180.0 / M_PI : -1.0;

  ldmx_log(trace) << "   Build Radii of containment (ROC)";

  auto trajectories = std::chrono::high_resolution_clock::now();
  profiling_map_["trajectories"] +=
      std::chrono::duration<float, std::milli>(trajectories - recoil_electron)
          .count();

  // Use the appropriate containment radii for the recoil electron
  std::vector<float> roc_values_bin0(roc_range_values_[0].begin() + 4,
                                     roc_range_values_[0].end());
  std::vector<float> ele_radii = roc_values_bin0;
  float theta_min, theta_max, p_min, p_max;
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
      std::vector<float> roc_values_bini(roc_range_values_[i].begin() + 4,
                                         roc_range_values_[i].end());
      ele_radii = roc_values_bini;
    }
  }
  // Use default RoC bin for photon
  std::vector<float> photon_radii = roc_values_bin0;

  auto roc_var = std::chrono::high_resolution_clock::now();
  profiling_map_["roc_var"] +=
      std::chrono::duration<float, std::milli>(roc_var - trajectories).count();

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

  auto fill_hitmaps = std::chrono::high_resolution_clock::now();
  profiling_map_["fill_hitmaps"] +=
      std::chrono::duration<float, std::milli>(fill_hitmaps - roc_var).count();

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

  auto containment_var = std::chrono::high_resolution_clock::now();
  profiling_map_["containment_var"] +=
      std::chrono::duration<float, std::milli>(containment_var - fill_hitmaps)
          .count();

  // MIP tracking:  vector of hits to be used in the MIP tracking algorithm. All
  // hits inside the electron ROC (or all hits in the ECal if the event is
  // missing an electron) will be included.
  std::vector<HitData> trackingHitList;

  ldmx_log(trace)
      << "   Loop over the hits from the event to calculate the BDT features";

  for (const ldmx::EcalHit &hit : ecalRecHits) {
    // Layer-wise quantities
    ldmx::EcalID id(hit.getID());
    ecalLayerEdepRaw_[id.layer()] =
        ecalLayerEdepRaw_[id.layer()] + hit.getEnergy();
    if (id.layer() >= 20) ecalBackEnergy_ += hit.getEnergy();
    if (maxCellDep_ < hit.getEnergy()) maxCellDep_ = hit.getEnergy();
    if (hit.getEnergy() <= 0) {
      ldmx_log(fatal)
          << "ECal hit has negative or zero energy, this should never happen, "
             "check the threshold settings in HgcrocEmulator";
      continue;
    }
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
          distance_photon_trajectory > (ireg + 1) * photon_radii[id.layer()]) {
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
  }  // end loop over rechits

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
  }  // end loop over rechits (2nd time)

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

  ldmx_log(trace) << "   Find out if the recoil electron is fiducial";

  // Find the location of the recoil electron
  // Ecal face is not where the first layer starts,
  // defined in DetDescr/python/EcalGeometry.py
  const float dz_from_face{7.932};
  float drifted_recoil_x{-9999.};
  float drifted_recoil_y{-9999.};
  if (recoilP[2] > 0.) {
    ldmx_log(trace) << "   Recoil electron pX = " << recoilP[0]
                    << " pY = " << recoilP[1] << " pZ = " << recoilP[2];
    ldmx_log(trace) << "   Recoil electron PosX = " << recoilPos[0]
                    << " PosY = " << recoilPos[1] << " PosZ = " << recoilPos[2];
    drifted_recoil_x =
        (dz_from_face * (recoilP[0] / recoilP[2])) + recoilPos[0];
    drifted_recoil_y =
        (dz_from_face * (recoilP[1] / recoilP[2])) + recoilPos[1];
  }
  const int recoil_layer_index = 0;

  // Check if it's fiducial
  bool inside_ecal_cell{false};
  // At module level
  const auto ecalID = geometry_->getID(drifted_recoil_x, drifted_recoil_y,
                                       recoil_layer_index, true);
  if (!ecalID.null()) {
    // If fiducial at module level, check at cell level
    const auto cellID =
        geometry_->getID(drifted_recoil_x, drifted_recoil_y, recoil_layer_index,
                         ecalID.getModuleID(), true);
    if (!cellID.null()) {
      inside_ecal_cell = true;
    }
  }

  ldmx_log(info) << "   Is this event is fiducial in ECAL? "
                 << inside_ecal_cell;

  event.add(trackingHitCollection, trackingHitList);
// Took out MIP tracking here

  auto linreg_tracks = std::chrono::high_resolution_clock::now();
  profiling_map_["linreg_tracks"] +=
      std::chrono::duration<float, std::milli>(linreg_tracks - straight_tracks)
          .count();

  ldmx_log(info) << " MIP tracking completed; found " << nStraightTracks_
                 << " straight tracks and " << nLinregTracks_
                 << " lin-reg tracks";

  result.setVariables(
      nReadoutHits_, deepestLayerHit_, summedDet_, summedTightIso_, maxCellDep_,
      showerRMS_, xStd_, yStd_, avgLayerHit_, stdLayerHit_, ecalBackEnergy_,
      nStraightTracks_, nLinregTracks_, firstNearPhLayer_, nNearPhHits_,
      photonTerritoryHits_, epAng_, epAngAtTarget_, epSep_, epDot_,
      epDotAtTarget_, electronContainmentEnergy, photonContainmentEnergy,
      outsideContainmentEnergy, outsideContainmentNHits, outsideContainmentXstd,
      outsideContainmentYstd, energySeg, xMeanSeg, yMeanSeg, xStdSeg, yStdSeg,
      layerMeanSeg, layerStdSeg, eContEnergy, eContXMean, eContYMean,
      gContEnergy, gContNHits, gContXMean, gContYMean, oContEnergy, oContNHits,
      oContXMean, oContYMean, oContXStd, oContYStd, oContLayerMean,
      oContLayerStd, ecalLayerEdepReadout_, recoilP, recoilPos);

  auto set_variables = std::chrono::high_resolution_clock::now();
  profiling_map_["set_variables"] +=
      std::chrono::duration<float, std::milli>(set_variables - linreg_tracks)
          .count();

  buildBDTFeatureVector(result);
  ldmx::Ort::FloatArrays inputs({bdtFeatures_});
  float pred = rt_->run({featureListName_}, inputs, {"probabilities"})[0].at(1);
  ldmx_log(info) << " BDT was ran, score is " << pred;
  // Other considerations were (nLinregTracks_ == 0)  && (firstNearPhLayer_ >=
  // 6)
  // && (epAng_ > 3.0 && epAng_ < 900 || epSep_ > 10.0 && epSep_ < 900)
  bool passesTrackingVeto = (nStraightTracks_ < 3);
  result.setVetoResult(pred > bdtCutVal_ && passesTrackingVeto);
  result.setDiscValue(pred);
  ldmx_log(info) << " The pred > bdtCutVal = " << (pred > bdtCutVal_)
                 << " and MIP tracking passed = " << passesTrackingVeto;

  // Persist in the event if the recoil ele is fiducial
  result.setFiducial(inside_ecal_cell);
  result.setTrackingFiducial(fiducial_in_tracker);

  // If the event passes the veto, keep it. Otherwise,
  // drop the event.
  if (!inverse_skim_) {
    if (result.passesVeto()) {
      setStorageHint(framework::hint_shouldKeep);
    } else {
      setStorageHint(framework::hint_shouldDrop);
    }
  } else {
    // Invert the skimming logic
    if (result.passesVeto()) {
      setStorageHint(framework::hint_shouldDrop);
    } else {
      setStorageHint(framework::hint_shouldKeep);
    }
  }

  event.add(collectionName_, result);

  auto bdt_variables = std::chrono::high_resolution_clock::now();
  profiling_map_["bdt_variables"] +=
      std::chrono::duration<float, std::milli>(bdt_variables - set_variables)
          .count();

  auto end = std::chrono::high_resolution_clock::now();
  auto time_diff = end - start;
  processing_time_ +=
      std::chrono::duration<float, std::milli>(time_diff).count();
}

void EcalVetoProcessor::onProcessEnd() {
  ldmx_log(info) << "AVG Time/Event: " << std::fixed << std::setprecision(2)
                 << processing_time_ / nevents_ << " ms";

  ldmx_log(info) << "Breakdown::";

  ldmx_log(info) << "setup                  Avg Time/Event = " << std::fixed
                 << std::setprecision(3) << profiling_map_["setup"] / nevents_
                 << " ms";

  ldmx_log(info) << "recoil_electron        Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["recoil_electron"] / nevents_ << " ms";
  ldmx_log(info) << "trajectories           Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["trajectories"] / nevents_ << " ms";

  ldmx_log(info) << "roc_var                Avg Time/Event = " << std::fixed
                 << std::setprecision(3) << profiling_map_["roc_var"] / nevents_
                 << " ms";

  ldmx_log(info) << "fill_hitmaps           Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["fill_hitmaps"] / nevents_ << " ms";
  ldmx_log(info) << "containment_var        Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["containment_var"] / nevents_ << " ms";
  ldmx_log(info) << "mip_tracking_setup     Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["mip_tracking_setup"] / nevents_ << " ms";

  ldmx_log(info) << "straight_tracks        Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["straight_tracks"] / nevents_ << " ms";

  ldmx_log(info) << "linreg_tracks          Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["linreg_tracks"] / nevents_ << " ms";

  ldmx_log(info) << "set_variables           Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["set_variables"] / nevents_ << " ms";

  ldmx_log(info) << "bdt_variables           Avg Time/Event = " << std::fixed
                 << std::setprecision(3)
                 << profiling_map_["bdt_variables"] / nevents_ << " ms";
}
/* Function to calculate the energy weighted shower centroid */
ldmx::EcalID EcalVetoProcessor::GetShowerCentroidIDAndRMS(
    const std::vector<ldmx::EcalHit> &ecalRecHits, float &showerRMS) {
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
  // flatten
  return ldmx::EcalID(0, returnCellId.module(), returnCellId.cell());
}

/**
 * Function to load up empty vector of hit maps
 */
void EcalVetoProcessor::fillHitMap(
    const std::vector<ldmx::EcalHit> &ecalRecHits,
    std::map<ldmx::EcalID, float> &cellMap) {
  for (const ldmx::EcalHit &hit : ecalRecHits) {
    ldmx::EcalID id(hit.getID());
    cellMap.emplace(id, hit.getEnergy());
  }
}

void EcalVetoProcessor::fillIsolatedHitMap(
    const std::vector<ldmx::EcalHit> &ecalRecHits, ldmx::EcalID globalCentroid,
    std::map<ldmx::EcalID, float> &cellMap,
    std::map<ldmx::EcalID, float> &cellMapIso, bool doTight) {
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
      if (cellMap.find(cellNbrIds[k]) != cellMap.end()) {
        isolatedHit = std::make_pair(false, cellNbrIds[k]);
        break;
      }
    }
    if (!isolatedHit.first) {
      continue;
    }
    // Insert isolated hit
    cellMapIso.emplace(id, hit.getEnergy());
  }
}

/* Calculate where trajectory intersects ECAL layers using position and momentum
 * at scoring plane */
std::vector<std::pair<float, float>> EcalVetoProcessor::getTrajectory(
    std::array<float, 3> momentum, std::array<float, 3> position) {
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

std::vector<float> EcalVetoProcessor::trackProp(const ldmx::Tracks &tracks,
                                                ldmx::TrackStateType ts_type,
                                                const std::string &ts_title) {
  // Vector to hold the new track state variables
  std::vector<float> new_track_states;

  // Return if no tracks
  if (tracks.empty()) return new_track_states;

  // Otherwise loop on the tracks
  for (auto &track : tracks) {
    // Get track state for ts_type
    auto trk_ts = track.getTrackState(ts_type);
    // Continue if there's no value
    if (!trk_ts.has_value()) continue;
    ldmx::Track::TrackState &ecal_track_state = trk_ts.value();

    // Check that the track state is filled
    if (ecal_track_state.params.size() < 5) continue;

    float track_state_loc0 = static_cast<float>(ecal_track_state.params[0]);
    float track_state_loc1 = static_cast<float>(ecal_track_state.params[1]);
    // param 2 = phi (azimuthal), param 3 = theta (polar)
    // param 4 = QoP
    // ACTS (local)  to  LDMX (global) coordinates: (y,z,x)->  (x,y,z)
    // convert qop [1/GeV] to p [MeV]
    float p_track_state = (-1 / ecal_track_state.params[4]) * 1000;
    // p * sin(theta) * sin(phi)
    float recoil_mom_x = p_track_state * sin(ecal_track_state.params[3]) *
                         sin(ecal_track_state.params[2]);
    // p * cos(theta)
    float recoil_mom_y = p_track_state * cos(ecal_track_state.params[3]);
    // p * sin(theta) * cos(phi)
    float recoil_mom_z = p_track_state * sin(ecal_track_state.params[3]) *
                         cos(ecal_track_state.params[2]);

    // Store the new track state variables
    new_track_states.push_back(track_state_loc0);
    new_track_states.push_back(track_state_loc1);
    // z-position at the ECAL (4) or Target (1)
    if (ts_type == 4) {
      // this should match `ECAL_SCORING_PLANE` in CKFProcessor
      new_track_states.push_back(240.5);
    } else if (ts_type == 1) {
      // This should match `target_surface` in CKFProcessor
      new_track_states.push_back(0.0);
    }

    new_track_states.push_back(recoil_mom_x);
    new_track_states.push_back(recoil_mom_y);
    new_track_states.push_back(recoil_mom_z);

    // Break after getting the first valid track state
    // TODO: interface this with CLUE to make sure the propageted track
    //       has an associated cluster in the ECAL
    break;
  }

  return new_track_states;
}

}  // namespace ecal

DECLARE_PRODUCER(ecal::EcalVetoProcessor);
