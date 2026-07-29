/**
 * @file EcalTrackFinderProcessor.cxx
 * @brief Implementation of ACTS-based ECAL track finder
 */

#include "Ecal/EcalTrackFinderProcessor.h"

// LDMX
#include "Tracking/Sim/MeasurementCalibrator.h"
#include "Tracking/Sim/TrackingUtils.h"
#include "Tracking/geo/CalibrationContext.h"
#include "Tracking/geo/GeometryContext.h"
#include "Tracking/geo/MagneticFieldContext.h"

// ACTS
#include "Acts/Definitions/Units.hpp"
#include "Acts/EventData/BoundTrackParameters.hpp"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/TransformationHelpers.hpp"
#include "Acts/Geometry/CuboidVolumeBuilder.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/TrackingGeometryBuilder.hpp"
#include "Acts/Geometry/TrackingVolume.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Propagator/ActorList.hpp"
#include "Acts/Propagator/MaterialInteractor.hpp"
#include "Acts/Propagator/StandardAborters.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/TrackFinding/MeasurementSelector.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Utilities/TrackHelpers.hpp"

// C++
#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <sstream>

namespace ecal {

EcalTrackFinderProcessor::EcalTrackFinderProcessor(const std::string& name,
                                                   framework::Process& process)
    : Producer(name, process) {
  // Setup surface rotation: u=+Y, v=+Z, w=+X (beam direction)
  surf_rotation_ = Acts::RotationMatrix3::Zero();
  surf_rotation_(1, 0) = 1;  // u along Y
  surf_rotation_(2, 1) = 1;  // v along Z
  surf_rotation_(0, 2) = 1;  // w along X (beam/normal)
}

void EcalTrackFinderProcessor::configure(
    framework::config::Parameters& parameters) {
  rec_coll_name_ = parameters.get<std::string>("rec_coll_name");
  rec_pass_name_ = parameters.get<std::string>("rec_pass_name");
  out_track_collection_ = parameters.get<std::string>("out_track_collection");

  min_hits_ = parameters.get<int>("min_hits");
  max_chi2_ = parameters.get<double>("max_chi2");
  cell_resolution_ = parameters.get<double>("cell_resolution");
  debug_ = parameters.get<bool>("debug");

  max_seed_rms_ = parameters.get<double>("max_seed_rms");
  min_momentum_ = parameters.get<double>("min_momentum");
  max_momentum_ = parameters.get<double>("max_momentum");

  use_roc_energy_ = parameters.get<bool>("use_roc_energy");
  if (use_roc_energy_) {
    roc_file_name_ = parameters.get<std::string>("roc_file");
    std::ifstream rocfile(roc_file_name_);
    if (!rocfile.good()) {
      EXCEPTION_RAISE("EcalTrackFinderProcessor",
                      "ROC file '" + roc_file_name_ + "' does not exist!");
    }
    std::string line, value;
    // Skip header line
    std::getline(rocfile, line);
    while (std::getline(rocfile, line)) {
      std::stringstream ss(line);
      std::vector<float> values;
      while (std::getline(ss, value, ',')) {
        values.push_back(value.empty() ? -1.0f : std::stof(value));
      }
      roc_range_values_.push_back(values);
    }
    ldmx_log(info) << "Loaded ROC file with " << roc_range_values_.size()
                   << " bins";
  }
}

void EcalTrackFinderProcessor::onNewRun(const ldmx::RunHeader&) {
  // Geometry is available here: EcalGeometryProvider::onNewRun has already
  // populated detector_geometry_ before processors receive onNewRun.
  geometry_ = &getCondition<ldmx::EcalGeometry>(
      ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  // Create ECAL layer surfaces
  layer_surfaces_.clear();
  createEcalSurfaces();

  // Setup zero magnetic field
  Acts::Vector3 b_field(0., 0., 0.);
  auto zero_b_field = std::make_shared<Acts::ConstantBField>(b_field);

  // Build ACTS tracking geometry for the ECAL using CuboidVolumeBuilder
  // Each ECAL layer becomes a sensitive layer in the tracking geometry

  auto& gctx = getCondition<tracking::geo::GeometryContext>(
                   tracking::geo::GeometryContext::NAME)
                   .get();

  // Get ECAL extent in ACTS coordinates
  double ecal_front_z = geometry_->getEcalFrontZ();
  double ecal_back_z =
      geometry_->getZPosition(geometry_->getNumLayers() - 1) + 50.0;
  Acts::Vector3 front_acts =
      tracking::sim::utils::ldmx2Acts(Acts::Vector3(0.0, 0.0, ecal_front_z));
  Acts::Vector3 back_acts =
      tracking::sim::utils::ldmx2Acts(Acts::Vector3(0.0, 0.0, ecal_back_z));

  // Create layer configurations - one per ECAL layer.
  // IMPORTANT: layer_configs must be in ascending x-order so LayerArrayCreator
  // gets a monotone sequence for BinningType::arbitrary along AxisX.
  std::vector<Acts::CuboidVolumeBuilder::LayerConfig> layer_configs;
  double clearance = 1.0;  // mm envelope around each layer surface

  for (auto& [layer, surface] : layer_surfaces_) {
    Acts::CuboidVolumeBuilder::LayerConfig lcfg;
    lcfg.surfaces = {surface};
    lcfg.envelopeX = std::array<double, 2>{clearance, clearance};
    lcfg.active = true;
    layer_configs.push_back(lcfg);
  }

  // Volume config
  Acts::Vector3 volume_center = 0.5 * (front_acts + back_acts);
  double x_length = std::abs(back_acts.x() - front_acts.x()) + 20.0;

  Acts::CuboidVolumeBuilder::VolumeConfig ecal_vol_cfg;
  ecal_vol_cfg.position = volume_center;
  ecal_vol_cfg.length = {x_length, 1000.0, 1000.0};  // generous transverse size
  ecal_vol_cfg.name = "EcalVolume";
  ecal_vol_cfg.layerCfg = layer_configs;
  ecal_vol_cfg.volumeMaterial =
      std::make_shared<Acts::HomogeneousVolumeMaterial>(
          Acts::Material::Vacuum());

  // Build the tracking geometry
  Acts::CuboidVolumeBuilder cvb;
  Acts::CuboidVolumeBuilder::Config cvb_cfg;
  cvb_cfg.position = volume_center;
  cvb_cfg.length = {x_length + 20.0, 1020.0, 1020.0};
  cvb_cfg.volumeCfg = {ecal_vol_cfg};
  cvb.setConfig(cvb_cfg);

  Acts::TrackingGeometryBuilder::Config tgb_cfg;
  tgb_cfg.trackingVolumeBuilders.push_back(
      [=](const auto& cxt, const auto& inner, const auto&) {
        return cvb.trackingVolume(cxt, inner, nullptr);
      });

  Acts::TrackingGeometryBuilder tgb(tgb_cfg);
  tracking_geometry_ = tgb.trackingGeometry(gctx);

  // Extract the geometry IDs that the builder assigned to our surfaces.
  // CuboidVolumeBuilder/TrackingGeometryBuilder reassign GeometryIdentifiers
  // based on the volume/layer/sensitive hierarchy, overwriting our manual IDs.
  // We must use these builder-assigned IDs when creating the source link map,
  // because the CKF Navigator will see these IDs when it reaches a surface.
  layer_geo_ids_.clear();
  tracking_geometry_->visitSurfaces([&](const Acts::Surface* surface) {
    if (!surface) return;
    // Only care about sensitive surfaces (those with a sensitive ID)
    if (surface->geometryId().sensitive() == 0) return;

    // CuboidVolumeBuilder creates new PlaneSurface objects inside the geometry
    // rather than using our originals. Mark them sensitive here so the CKF
    // actor doesn't skip them as passive surfaces.
    const_cast<Acts::Surface*>(surface)->assignIsSensitive(true);

    // Match to ECAL layer by z position (LDMX frame)
    Acts::Vector3 center_ldmx =
        tracking::sim::utils::acts2Ldmx(surface->center(gctx));
    double z_ldmx = center_ldmx[2];

    for (int layer = 0; layer < geometry_->getNumLayers(); ++layer) {
      double layer_z = geometry_->getZPosition(layer);
      if (std::abs(z_ldmx - layer_z) < 0.1) {  // 0.1 mm tolerance
        layer_geo_ids_[layer] = surface->geometryId();
        ldmx_log(debug) << "ECAL layer " << layer << " -> builder geo_id: vol="
                        << surface->geometryId().volume()
                        << " lay=" << surface->geometryId().layer()
                        << " sen=" << surface->geometryId().sensitive();
        break;
      }
    }
  });

  ldmx_log(info) << "Mapped " << layer_geo_ids_.size()
                 << " ECAL layers to builder-assigned geometry IDs";

  // Setup stepper with zero B-field
  const auto stepper = Acts::EigenStepper<>{zero_b_field};

  auto acts_logging_level =
      debug_ ? Acts::Logging::VERBOSE : Acts::Logging::FATAL;

  // Setup navigator with tracking geometry
  Acts::Navigator::Config nav_cfg{tracking_geometry_};
  nav_cfg.resolveSensitive = true;
  nav_cfg.resolvePassive = false;
  nav_cfg.resolveMaterial = false;
  const Acts::Navigator navigator(
      nav_cfg,
      Acts::getDefaultLogger("ECAL_NAV", acts_logging_level));

  // Create propagator
  propagator_ = std::make_unique<EcalPropagator>(
      stepper, navigator,
      Acts::getDefaultLogger("ECAL_PROP", acts_logging_level));

  // Create CKF
  ckf_ = std::make_unique<std::decay_t<decltype(*ckf_)>>(
      *propagator_, Acts::getDefaultLogger("ECAL_CKF", acts_logging_level));

  ldmx_log(info) << "EcalTrackFinderProcessor initialized with "
                 << layer_surfaces_.size() << " ECAL layer surfaces";
}

void EcalTrackFinderProcessor::createEcalSurfaces() {
  int n_layers = geometry_->getNumLayers();

  for (int layer = 0; layer < n_layers; ++layer) {
    // Get z position of this layer
    double z_pos = geometry_->getZPosition(layer);

    // Create plane surface at this z position
    // Position in ACTS frame (x=z_ldmx, y=x_ldmx, z=y_ldmx)
    Acts::Vector3 acts_pos =
        tracking::sim::utils::ldmx2Acts(Acts::Vector3(0.0, 0.0, z_pos));

    Acts::Translation3 translation(acts_pos);
    Acts::Transform3 transform(translation * surf_rotation_);

    // Create bounded plane surface (500mm x 500mm, covers full ECAL)
    auto bounds = std::make_shared<Acts::RectangleBounds>(500.0, 500.0);
    auto surface =
        Acts::Surface::makeShared<Acts::PlaneSurface>(transform, bounds);

    // Mark as sensitive so the CKF actor creates track states here.
    // PlaneSurfaces default to isSensitive()=false; without this flag the CKF
    // treats them as passive material surfaces and creates no track states.
    surface->assignIsSensitive(true);

    // Assign a geometry ID (use layer as volume, 0 as layer in ACTS sense)
    Acts::GeometryIdentifier geo_id =
        Acts::GeometryIdentifier().withVolume(layer).withLayer(0);
    surface->assignGeometryId(geo_id);

    layer_surfaces_[layer] = surface;
  }

  // Create reference surface at ECAL front face
  double ecal_front_z = geometry_->getEcalFrontZ();
  Acts::Vector3 ref_pos =
      tracking::sim::utils::ldmx2Acts(Acts::Vector3(0.0, 0.0, ecal_front_z));
  Acts::Translation3 ref_translation(ref_pos);
  Acts::Transform3 ref_transform(ref_translation * surf_rotation_);
  reference_surface_ =
      Acts::Surface::makeShared<Acts::PlaneSurface>(ref_transform);
}

std::vector<ldmx::Measurement> EcalTrackFinderProcessor::createMeasurements(
    const std::vector<ldmx::EcalHit>& hits, std::vector<double>& energies) {
  std::vector<ldmx::Measurement> measurements;
  measurements.reserve(hits.size());
  energies.clear();
  energies.reserve(hits.size());

  for (const auto& hit : hits) {
    // Skip noise hits
    if (hit.isNoise()) continue;

    // Get EcalID
    ldmx::EcalID ecal_id(hit.getID());
    int layer = ecal_id.layer();

    // Get position from geometry
    auto [x, y, z] = geometry_->getPosition(ecal_id);

    // Create measurement
    ldmx::Measurement meas;
    meas.setGlobalPosition(x, y, z);
    meas.setLayerID(layer);
    meas.setTime(hit.getTime());

    // Local coordinates: use x, y as local u, v in the layer plane
    meas.setLocalPosition(x, y);

    // Covariance: use cell resolution
    double cov = cell_resolution_ * cell_resolution_;
    meas.setLocalCovariance(cov, cov);

    measurements.push_back(meas);
    energies.push_back(hit.getEnergy());
  }

  return measurements;
}

std::tuple<Acts::Vector3, Acts::Vector3, double>
EcalTrackFinderProcessor::fitStraightLine(
    const std::vector<Acts::Vector3>& points) {
  if (points.size() < 2) {
    return {Acts::Vector3::Zero(), Acts::Vector3::Zero(), 1e6};
  }

  // Calculate centroid
  Acts::Vector3 centroid = Acts::Vector3::Zero();
  for (const auto& p : points) {
    centroid += p;
  }
  centroid /= points.size();

  // Build covariance matrix
  Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
  for (const auto& p : points) {
    Acts::Vector3 dp = p - centroid;
    cov += dp * dp.transpose();
  }
  cov /= points.size();

  // Find principal direction (eigenvector with largest eigenvalue)
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov);
  Acts::Vector3 direction = solver.eigenvectors().col(2);  // Largest eigenvalue
  direction.normalize();

  // Eigenvector has sign ambiguity — ensure it points forward along the beam.
  // In ACTS coords, beam direction is +X (LDMX Z -> ACTS X).
  if (direction.x() < 0) {
    direction = -direction;
  }

  // Calculate RMS residual
  double rms = 0.0;
  for (const auto& p : points) {
    Acts::Vector3 dp = p - centroid;
    double residual = (dp - (dp.dot(direction)) * direction).norm();
    rms += residual * residual;
  }
  rms = std::sqrt(rms / points.size());

  return {centroid, direction, rms};
}

std::vector<ldmx::Track> EcalTrackFinderProcessor::findSeeds(
    const std::vector<ldmx::Measurement>& measurements) {
  std::vector<ldmx::Track> seeds;

  if (measurements.size() < static_cast<size_t>(min_hits_)) {
    ldmx_log(debug) << "Too few measurements for seed: " << measurements.size()
                    << " < " << min_hits_;
    return seeds;
  }

  // Group measurements by layer
  std::map<int, std::vector<const ldmx::Measurement*>> layer_map;
  for (const auto& meas : measurements) {
    layer_map[meas.getLayerID()].push_back(&meas);
  }

  ldmx_log(debug) << "Measurements span " << layer_map.size() << " layers";

  // Simple strategy: use all hits for a single seed (can be extended later)
  std::vector<Acts::Vector3> points;
  std::vector<ldmx::Measurement> seed_measurements;

  for (const auto& [layer, meas_vec] : layer_map) {
    // For now, just take first hit in each layer
    if (!meas_vec.empty()) {
      const auto* meas = meas_vec[0];
      auto gpos = meas->getGlobalPosition();

      // Convert to ACTS frame
      Acts::Vector3 pos_acts = tracking::sim::utils::ldmx2Acts(
          Acts::Vector3(gpos[0], gpos[1], gpos[2]));
      points.push_back(pos_acts);
      seed_measurements.push_back(*meas);
    }
  }

  if (points.size() < static_cast<size_t>(min_hits_)) {
    ldmx_log(debug) << "Too few layers hit for seed: " << points.size() << " < "
                    << min_hits_;
    return seeds;
  }

  // Fit straight line
  auto [position, direction, rms] = fitStraightLine(points);

  ldmx_log(debug) << "Seed line fit: rms=" << rms << " dir=(" << direction.x()
                  << "," << direction.y() << "," << direction.z() << ")";

  if (rms > max_seed_rms_) {
    ldmx_log(debug) << "Seed RMS too large: " << rms << " > " << max_seed_rms_
                    << " mm";
    return seeds;
  }

  // Create seed track at the FIRST ECAL layer surface (layer 0).
  // Using a surface that IS in the tracking geometry (has associatedLayer set)
  // ensures the ACTS Navigator can use the fast initialization path and find
  // all sensitive surfaces during CKF propagation.
  auto& gctx = getCondition<tracking::geo::GeometryContext>(
                   tracking::geo::GeometryContext::NAME)
                   .get();

  auto& seed_surface = layer_surfaces_.begin()->second;

  // For a plane surface, normal is the third column of rotation
  Acts::Vector3 ref_normal =
      seed_surface->localToGlobalTransform(gctx).rotation().col(2);
  Acts::Vector3 ref_center = seed_surface->center(gctx);

  double t =
      (ref_center - position).dot(ref_normal) / direction.dot(ref_normal);
  Acts::Vector3 seed_pos = position + t * direction;

  // Estimate momentum (assume MIP ~200 MeV for now)
  double p_estimate = 200.0;  // MeV
  Acts::Vector3 seed_mom = p_estimate * direction;

  // Charge (assume positive)
  double q = Acts::UnitConstants::e;

  // Convert to bound parameters at the first layer surface
  Acts::FreeVector seed_free =
      tracking::sim::utils::toFreeParameters(seed_pos, seed_mom, q);

  auto bound_params_result = Acts::transformFreeToBoundParameters(
      seed_free, *seed_surface, gctx);

  if (!bound_params_result.ok()) {
    ldmx_log(warn) << "Failed to create bound parameters for seed";
    return seeds;
  }

  Acts::BoundVector bound_params = bound_params_result.value();

  // Create inflated covariance
  Acts::BoundVector stddev;
  stddev[Acts::eBoundLoc0] = 10.0 * Acts::UnitConstants::mm;
  stddev[Acts::eBoundLoc1] = 10.0 * Acts::UnitConstants::mm;
  stddev[Acts::eBoundPhi] = 0.1 * Acts::UnitConstants::rad;
  stddev[Acts::eBoundTheta] = 0.1 * Acts::UnitConstants::rad;
  stddev[Acts::eBoundQOverP] = 0.5 / p_estimate;  // 50% uncertainty
  stddev[Acts::eBoundTime] = 10.0 * Acts::UnitConstants::ns;

  Acts::BoundMatrix bound_cov = stddev.cwiseProduct(stddev).asDiagonal();

  // Create ldmx::Track seed
  ldmx::Track seed;

  // Convert layer 0 surface position to LDMX frame
  Acts::Vector3 ref_ldmx = tracking::sim::utils::acts2Ldmx(ref_center);
  seed.setPerigeeLocation(ref_ldmx[0], ref_ldmx[1], ref_ldmx[2]);

  seed.setChi2(0.0);
  seed.setNhits(seed_measurements.size());
  seed.setNdf(0);
  seed.setNsharedHits(0);
  seed.setCharge(q > 0 ? 1 : -1);

  // Convert to std::vector for storage
  std::vector<double> v_seed_params(bound_params.data(),
                                    bound_params.data() + bound_params.size());
  std::vector<double> v_seed_cov;
  tracking::sim::utils::flatCov(bound_cov, v_seed_cov);

  seed.setPerigeeParameters(v_seed_params);
  seed.setPerigeeCov(v_seed_cov);

  seeds.push_back(seed);
  return seeds;
}

std::unordered_multimap<Acts::GeometryIdentifier,
                        acts_examples::IndexSourceLink>
EcalTrackFinderProcessor::makeGeoIdSourceLinkMap(
    const std::vector<ldmx::Measurement>& measurements) {
  std::unordered_multimap<Acts::GeometryIdentifier,
                          acts_examples::IndexSourceLink>
      geo_id_sl_map;

  for (size_t i = 0; i < measurements.size(); ++i) {
    const auto& meas = measurements[i];
    int layer = meas.getLayerID();

    // Use the builder-assigned geometry ID for this layer (not the manually
    // assigned one). This ensures the CKF Navigator's surface.geometryId()
    // matches our source link map keys.
    auto it = layer_geo_ids_.find(layer);
    if (it == layer_geo_ids_.end()) {
      ldmx_log(warn) << "No builder geometry ID for layer " << layer;
      continue;
    }

    Acts::GeometryIdentifier geo_id = it->second;
    acts_examples::IndexSourceLink idx_sl(geo_id, i);
    geo_id_sl_map.insert(std::make_pair(geo_id, idx_sl));
  }

  ldmx_log(debug) << "Source link map has " << geo_id_sl_map.size()
                  << " entries from " << measurements.size() << " measurements";

  return geo_id_sl_map;
}

void EcalTrackFinderProcessor::produce(framework::Event& event) {
  auto start = std::chrono::high_resolution_clock::now();
  nevents_++;

  std::vector<ldmx::Track> tracks;

  // Get ECAL RecHits
  if (!event.exists(rec_coll_name_, rec_pass_name_)) {
    ldmx_log(debug) << "No ECAL RecHits collection found";
    event.add(out_track_collection_, tracks);
    return;
  }

  const std::vector<ldmx::EcalHit> ecal_hits =
      event.getCollection<ldmx::EcalHit>(rec_coll_name_, rec_pass_name_);

  ldmx_log(debug) << "Processing " << ecal_hits.size() << " ECAL hits";

  // Convert hits to measurements (with parallel energy vector)
  std::vector<double> measurement_energies;
  auto measurements = createMeasurements(ecal_hits, measurement_energies);
  ldmx_log(debug) << "Created " << measurements.size() << " measurements";

  if (measurements.empty()) {
    event.add(out_track_collection_, tracks);
    return;
  }

  // Create source link map
  auto geo_id_sl_map = makeGeoIdSourceLinkMap(measurements);
  ldmx_log(info) << "Source link map: " << geo_id_sl_map.size()
                 << " entries from " << measurements.size()
                 << " measurements";

  // Find seed tracks
  auto seed_tracks = findSeeds(measurements);
  ldmx_log(info) << "Found " << seed_tracks.size() << " seed tracks";
  nseeds_ += seed_tracks.size();

  if (seed_tracks.empty()) {
    event.add(out_track_collection_, tracks);
    return;
  }

  // Get ACTS contexts
  auto& gctx = getCondition<tracking::geo::GeometryContext>(
                   tracking::geo::GeometryContext::NAME)
                   .get();
  auto& mctx = getCondition<tracking::geo::MagneticFieldContext>(
                   tracking::geo::MagneticFieldContext::NAME)
                   .get();
  auto& cctx = getCondition<tracking::geo::CalibrationContext>(
                   tracking::geo::CalibrationContext::NAME)
                   .get();

  // Setup propagator options
  Acts::PropagatorPlainOptions propagator_options(gctx, mctx);
  propagator_options.pathLimit = std::numeric_limits<double>::max();
  propagator_options.maxSteps = 1000;
  propagator_options.stepping.maxStepSize = 100.0 * Acts::UnitConstants::mm;

  // Setup CKF extensions
  Acts::GainMatrixUpdater kf_updater;
  Acts::MeasurementSelector::Config meas_sel_cfg = {
      {Acts::GeometryIdentifier(), {{}, {max_chi2_}, {1u}}}};
  Acts::MeasurementSelector meas_sel{meas_sel_cfg};

  tracking::sim::LdmxMeasurementCalibrator calibrator{measurements};

  // Setup source link accessor iterator type and lambda
  struct SourceLinkAccIt {
    using BaseIt = decltype(geo_id_sl_map.begin());
    BaseIt it_;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"

    using difference_type = typename BaseIt::difference_type;
    using iterator_category = std::input_iterator_tag;
    using value_type = Acts::SourceLink;
    using pointer = value_type*;
    using reference = value_type&;
#pragma GCC diagnostic pop

    SourceLinkAccIt& operator++() {
      ++it_;
      return *this;
    }
    bool operator==(const SourceLinkAccIt& other) const {
      return it_ == other.it_;
    }
    bool operator!=(const SourceLinkAccIt& other) const {
      return !(*this == other);
    }
    value_type operator*() const { return value_type{it_->second}; }
  };

  auto source_link_accessor = [&](const Acts::Surface& surface)
      -> std::pair<SourceLinkAccIt, SourceLinkAccIt> {
    auto [begin, end] = geo_id_sl_map.equal_range(surface.geometryId());
    return {SourceLinkAccIt{begin}, SourceLinkAccIt{end}};
  };

  // v46: calibrator and measurementSelector moved to TrackStateCreator
  Acts::TrackStateCreator<SourceLinkAccIt, TrackContainer> track_state_creator;
  track_state_creator.sourceLinkAccessor
      .connect<&decltype(source_link_accessor)::operator(),
               decltype(source_link_accessor)>(&source_link_accessor);
  track_state_creator.calibrator
      .connect<&tracking::sim::LdmxMeasurementCalibrator::calibrate<
          Acts::VectorMultiTrajectory>>(&calibrator);
  track_state_creator.measurementSelector
      .connect<&Acts::MeasurementSelector::select<Acts::VectorMultiTrajectory>>(
          &meas_sel);

  Acts::CombinatorialKalmanFilterExtensions<TrackContainer> ckf_extensions;
  ckf_extensions.updater.connect<
      &Acts::GainMatrixUpdater::operator()<Acts::VectorMultiTrajectory>>(
      &kf_updater);
  ckf_extensions.createTrackStates.connect<&Acts::TrackStateCreator<
      SourceLinkAccIt, TrackContainer>::createTrackStates>(
      &track_state_creator);

  // Create track container
  Acts::VectorTrackContainer vtc;
  Acts::VectorMultiTrajectory mtj;
  Acts::TrackContainer tc{vtc, mtj};

  // Process each seed
  for (size_t seed_idx = 0; seed_idx < seed_tracks.size(); ++seed_idx) {
    const auto& seed = seed_tracks[seed_idx];

    // Convert seed to BoundTrackParameters.
    // Start from layer_surfaces_[0] which is part of the tracking geometry and
    // has associatedLayer() set — this lets the Navigator use the fast
    // initialization path and correctly traverse all 32 ECAL layers.
    Acts::BoundVector param_vec;
    param_vec << seed.getD0(), seed.getZ0(), seed.getPhi(), seed.getTheta(),
        seed.getQoP(), seed.getT();

    ldmx_log(debug) << "Seed " << seed_idx << ": loc0=" << param_vec[0]
                    << " loc1=" << param_vec[1] << " phi=" << param_vec[2]
                    << " theta=" << param_vec[3] << " qop=" << param_vec[4];

    Acts::BoundMatrix cov_mat =
        tracking::sim::utils::unpackCov(seed.getPerigeeCov());

    auto part_hypo{Acts::ParticleHypothesis::electron()};
    auto& layer0_surface = layer_surfaces_.begin()->second;
    Acts::BoundTrackParameters start_params(layer0_surface, param_vec,
                                            cov_mat, part_hypo);

    // Setup CKF options
    const Acts::CombinatorialKalmanFilterOptions<TrackContainer> ckf_options(
        gctx, mctx, cctx, ckf_extensions, propagator_options);

    // Run CKF
    auto results = ckf_->findTracks(start_params, ckf_options, tc);

    if (!results.ok()) {
      ldmx_log(debug) << "CKF failed for seed " << seed_idx << ": "
                      << results.error().message();
      continue;
    }

    auto& tracks_from_seed = results.value();
    ldmx_log(info) << "CKF returned " << tracks_from_seed.size()
                   << " tracks from seed " << seed_idx;
    for (auto& track : tracks_from_seed) {
      // Count track state types before smoothing
      int n_meas = 0, n_holes = 0, n_outliers = 0, n_total = 0;
      for (const auto& ts : track.trackStatesReversed()) {
        ++n_total;
        if (ts.typeFlags().isMeasurement()) ++n_meas;
        if (ts.typeFlags().isHole()) ++n_holes;
        if (ts.typeFlags().isOutlier()) ++n_outliers;
      }
      ldmx_log(info) << "Track states: total=" << n_total
                     << " meas=" << n_meas << " holes=" << n_holes
                     << " outliers=" << n_outliers;

      // Smooth the track
      auto smooth_result = Acts::smoothTrack(gctx, track);
      if (!smooth_result.ok()) {
        ldmx_log(warn) << "smoothTrack failed: "
                       << smooth_result.error().message();
        continue;
      }

      // Create output track
      ldmx::Track trk;

      // Get parameters from first smoothed state
      // (setReferenceSurface doesn't actually transform params, need actual
      // state)
      Acts::BoundVector smoothed_params;
      std::shared_ptr<const Acts::Surface> smoothed_surface;
      bool found_smoothed = false;

      for (const auto& ts : track.trackStatesReversed()) {
        if (ts.hasSmoothed()) {
          smoothed_params = ts.smoothed();
          smoothed_surface = ts.referenceSurface().getSharedPtr();
          found_smoothed = true;
          break;
        }
      }

      if (!found_smoothed) {
        ldmx_log(warn) << "No smoothed track state found after smoothing";
        continue;
      }

      ldmx_log(debug) << "Smoothed params: loc0=" << smoothed_params[0]
                      << " loc1=" << smoothed_params[1]
                      << " phi=" << smoothed_params[2]
                      << " theta=" << smoothed_params[3]
                      << " qop=" << smoothed_params[4];

      // Convert to free parameters (in ACTS global coords)
      Acts::FreeVector free_params = Acts::transformBoundToFreeParameters(
          *smoothed_surface, gctx, smoothed_params);

      // Convert ACTS position and momentum to LDMX coordinates
      Acts::Vector3 pos_acts(free_params[Acts::eFreePos0],
                             free_params[Acts::eFreePos1],
                             free_params[Acts::eFreePos2]);
      Acts::Vector3 mom_acts(free_params[Acts::eFreeDir0],
                             free_params[Acts::eFreeDir1],
                             free_params[Acts::eFreeDir2]);

      Acts::Vector3 pos_ldmx = tracking::sim::utils::acts2Ldmx(pos_acts);
      Acts::Vector3 mom_ldmx = tracking::sim::utils::acts2Ldmx(mom_acts);

      double x = pos_ldmx[0];
      double y = pos_ldmx[1];
      double z = pos_ldmx[2];
      double px = mom_ldmx[0];
      double py = mom_ldmx[1];
      double pz = mom_ldmx[2];

      ldmx_log(debug) << "LDMX momentum: px=" << px << " py=" << py
                      << " pz=" << pz;

      // Compute theta and phi same way as SP electron (atan2(pt, pz))
      double pt = std::sqrt(px * px + py * py);
      double theta = std::atan2(pt, pz);
      double phi = std::atan2(py, px);
      if (phi < 0)
        phi += 2.0 * M_PI;  // Shift to [0, 2π] to avoid boundary artifacts
      double qop = free_params[Acts::eFreeQOverP];

      // Compute perigee parameters from smoothed track state position.
      // PCA-based z0 is ill-defined for forward tracks (pz >> pt), so
      // we just use the z of the smoothed state directly.
      double d0 = -(x * std::sin(phi) - y * std::cos(phi));
      double z0 = z;
      double time = free_params[Acts::eFreeTime];

      Acts::BoundVector perigee_params;
      perigee_params << d0, z0, phi, theta, qop, time;

      ldmx_log(debug) << "Perigee params: d0=" << d0 << " z0=" << z0
                      << " phi=" << phi << " theta=" << theta;

      // Store perigee parameters
      trk.setPerigeeParameters(
          tracking::sim::utils::convertActsToLdmxPars(perigee_params));

      // Covariance is always available for TrackProxy
      std::vector<double> cov_vec;
      tracking::sim::utils::flatCov(track.covariance(), cov_vec);
      trk.setPerigeeCov(cov_vec);

      Acts::Vector3 ref_loc_ldmx =
          tracking::sim::utils::acts2Ldmx(layer_surfaces_.begin()->second->center(gctx));
      trk.setPerigeeLocation(ref_loc_ldmx[0], ref_loc_ldmx[1], ref_loc_ldmx[2]);

      trk.setChi2(track.chi2());
      trk.setNhits(track.nMeasurements());
      trk.setNdf(track.nMeasurements() - 5);
      trk.setNsharedHits(0);
      trk.setCharge(qop > 0 ? 1 : -1);

      // Add measurement indices
      for (const auto ts : track.trackStatesReversed()) {
        if (ts.typeFlags().isMeasurement() && ts.hasUncalibratedSourceLink()) {
          Acts::SourceLink usl = ts.getUncalibratedSourceLink();
          const acts_examples::IndexSourceLink& sl =
              usl.get<acts_examples::IndexSourceLink>();
          trk.addMeasurementIndex(sl.index());
        }
      }

      // Compute track energy from RecHits
      double track_energy = 0.0;

      if (use_roc_energy_ && !roc_range_values_.empty()) {
        // Use 68% containment cone: project track through each layer,
        // sum energies of all RecHits within the ROC radius

        // Track momentum magnitude and angle for ROC bin selection
        double trk_p_mag = 1.0 / std::abs(smoothed_params[Acts::eBoundQOverP]);
        double trk_theta_deg = theta * 180.0 / M_PI;

        // Select ROC bin based on momentum and angle
        std::vector<float> ele_radii(roc_range_values_[0].begin() + 4,
                                     roc_range_values_[0].end());
        for (const auto& row : roc_range_values_) {
          float theta_min = row[0], theta_max = row[1];
          float p_min = row[2], p_max = row[3];
          bool inrange = true;
          if (theta_min != -1.0f)
            inrange = inrange && (trk_theta_deg >= theta_min);
          if (theta_max != -1.0f)
            inrange = inrange && (trk_theta_deg < theta_max);
          if (p_min != -1.0f) inrange = inrange && (trk_p_mag >= p_min);
          if (p_max != -1.0f) inrange = inrange && (trk_p_mag < p_max);
          if (inrange) {
            ele_radii.assign(row.begin() + 4, row.end());
          }
        }

        // Project track through each ECAL layer (straight line in LDMX coords)
        // Track at (x, y, z) with direction (px, py, pz) normalized
        for (const auto& hit : ecal_hits) {
          if (hit.isNoise()) continue;
          ldmx::EcalID ecal_id(hit.getID());
          int layer = ecal_id.layer();
          if (layer < 0 || layer >= static_cast<int>(ele_radii.size()))
            continue;

          auto [hx, hy, hz] = geometry_->getPosition(ecal_id);

          // Project track to this layer's z
          double dz = hz - z;
          double proj_x = x + (px / pz) * dz;
          double proj_y = y + (py / pz) * dz;

          // Distance from projected track to hit
          double dx = hx - proj_x;
          double dy = hy - proj_y;
          double dist = std::sqrt(dx * dx + dy * dy);

          if (dist < ele_radii[layer]) {
            track_energy += hit.getEnergy();
          }
        }
      } else {
        // Fallback: sum on-track hit energies only
        for (const auto ts : track.trackStatesReversed()) {
          if (ts.typeFlags().isMeasurement() &&
              ts.hasUncalibratedSourceLink()) {
            Acts::SourceLink usl = ts.getUncalibratedSourceLink();
            const acts_examples::IndexSourceLink& sl =
                usl.get<acts_examples::IndexSourceLink>();
            track_energy += measurement_energies[sl.index()];
          }
        }
      }

      // Use energy as track momentum (E ≈ p for relativistic electrons)
      double track_p = track_energy;
      qop = (track_p > 0) ? -1.0 / track_p : 0.0;
      perigee_params[Acts::eBoundQOverP] = qop;
      trk.setPerigeeParameters(
          tracking::sim::utils::convertActsToLdmxPars(perigee_params));

      ldmx_log(debug) << "Track energy from RecHits: " << track_energy
                      << " MeV, q/p=" << qop;

      tracks.push_back(trk);
      ntracks_++;
    }
  }

  ldmx_log(info) << "Found " << tracks.size() << " fitted tracks";

  // Add to event
  event.add(out_track_collection_, tracks);

  auto end = std::chrono::high_resolution_clock::now();
  auto diff = end - start;
  processing_time_ += std::chrono::duration<double, std::milli>(diff).count();
}

void EcalTrackFinderProcessor::onProcessEnd() {
  ldmx_log(info) << "EcalTrackFinderProcessor Statistics:";
  ldmx_log(info) << "  Events processed: " << nevents_;
  ldmx_log(info) << "  Total seeds: " << nseeds_;
  ldmx_log(info) << "  Total tracks: " << ntracks_;
  ldmx_log(info) << "  Avg tracks/event: "
                 << (nevents_ > 0 ? (double)ntracks_ / nevents_ : 0);
  ldmx_log(info) << "  Avg time/event: "
                 << (nevents_ > 0 ? processing_time_ / nevents_ : 0) << " ms";
}

}  // namespace ecal

DECLARE_PRODUCER(ecal::EcalTrackFinderProcessor)
