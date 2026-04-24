#include "Tracking/Reco/DigitizationProcessor.h"

#include <algorithm>

#include "Tracking/Digitization/ChargeCarrier.h"

using namespace framework;

namespace tracking::reco {

DigitizationProcessor::DigitizationProcessor(const std::string& name,
                                             framework::Process& process)
    : TrackingGeometryUser(name, process) {}

void DigitizationProcessor::onProcessStart() {
  normal_ = std::make_shared<std::normal_distribution<float>>(0., 1.);

  if (use_charge_digitization_) {
    strip_digitizer_ = std::make_unique<tracking::digitization::SiStripDigitizer>(
        sensor_params_, generator_);
    ldmx_log(info) << "Charge digitization enabled."
                   << "  thickness=from geometry"
                   << "  sense_pitch="     << tracking::digitization::SENSE_PITCH_MM   << " mm"
                   << "  readout_pitch="   << tracking::digitization::READOUT_PITCH_MM << " mm"
                   << "  Vbias="           << sensor_params_.bias_voltage      << " V"
                   << "  Vdep="            << sensor_params_.depletion_voltage << " V"
                   << "  bulk="            << (sensor_params_.is_n_type ? "n" : "p") << "-type"
                   << "  e_lorentz_tan="   << sensor_params_.electron_lorentz_tangent
                   << "  h_lorentz_tan="   << sensor_params_.hole_lorentz_tangent
                   << "  trapping="        << sensor_params_.trapping
                   << "  noise="           << sensor_params_.noise_electrons   << " e-"
                   << "  threshold="       << sensor_params_.threshold_electrons << " e-"
                   << "  n_segments_min="  << sensor_params_.n_segments_min
                   << "  granularity="     << sensor_params_.deposition_granularity;

    pulse_shape_ = tracking::digitization::PulseShape::make(
        std::string(tracking::digitization::PULSE_SHAPE_NAME),
        tracking::digitization::PEAKING_TIME_NS,
        tracking::digitization::SECOND_TIME_CONST_NS);
    ldmx_log(info) << "Pulse shaping: shape=" << tracking::digitization::PULSE_SHAPE_NAME
                   << "  tp=" << tracking::digitization::PEAKING_TIME_NS << " ns"
                   << "  n_samples=" << tracking::digitization::N_SAMPLES
                   << "  sampling_interval=" << tracking::digitization::SAMPLING_INTERVAL_NS << " ns"
                   << "  t0_offset=" << tracking::digitization::T0_OFFSET_NS << " ns";

    if (field_map_.empty()) {
      ldmx_log(debug) << "field_map not set; will auto-load from GDML";
    }
    buildLorentzCache();
  }
}

void DigitizationProcessor::configure(
    framework::config::Parameters& parameters) {
  hit_collection_ =
      parameters.get<std::string>("hit_collection", "TaggerSimHits");

  tracker_hit_passname_ = parameters.get<std::string>("tracker_hit_passname");
  out_collection_ =
      parameters.get<std::string>("out_collection", "OutputMeasuements");
  min_e_dep_ = parameters.get<double>("min_e_dep", 0.05);
  track_id_  = parameters.get<int>("track_id", -1);
  do_smearing_  = parameters.get<bool>("do_smearing", true);
  sigma_u_      = parameters.get<double>("sigma_u", 0.01);
  sigma_v_      = parameters.get<double>("sigma_v", 0.);
  merge_hits_   = parameters.get<bool>("merge_hits", false);

  // Mode 1: charge digitization parameters
  use_charge_digitization_ =
      parameters.get<bool>("use_charge_digitization", false);

  if (use_charge_digitization_) {
    sensor_params_.bias_voltage =
        parameters.get<double>("bias_voltage", 200.0);
    sensor_params_.depletion_voltage =
        parameters.get<double>("depletion_voltage", 70.0);
    sensor_params_.temperature =
        parameters.get<double>("temperature", 300.0);
    sensor_params_.noise_electrons =
        parameters.get<double>("noise_electrons", 1000.0);
    sensor_params_.threshold_electrons =
        parameters.get<double>("threshold_electrons", 3000.0);
    sensor_params_.is_n_type =
        parameters.get<bool>("is_n_type", false);
    sensor_params_.electron_side_readout =
        parameters.get<bool>("electron_side_readout", false);
    sensor_params_.hole_side_readout =
        parameters.get<bool>("hole_side_readout", true);
    sensor_params_.electron_lorentz_tangent =
        parameters.get<double>("electron_lorentz_tangent", 0.0);
    sensor_params_.hole_lorentz_tangent =
        parameters.get<double>("hole_lorentz_tangent", 0.0);
    sensor_params_.trapping =
        parameters.get<double>("trapping", 0.0);
    sensor_params_.deposition_granularity =
        parameters.get<double>("deposition_granularity", 0.10);
    sensor_params_.n_segments_min =
        parameters.get<int>("n_segments_min", 5);
    sensor_params_.n_readout_strips =
        parameters.get<int>("n_readout_strips", 767);

    out_raw_collection_ =
        parameters.get<std::string>("out_raw_collection", "");
    field_map_ =
        parameters.get<std::string>("field_map", "");
  }
}

void DigitizationProcessor::buildLorentzCache() {
  if (!use_charge_digitization_) return;

  if (field_map_.empty())
    loadBField();
  else
    loadBField(field_map_);

  // Low-field (Hall) mobility from the Canali model [cm²/(V·s)] → [m²/(V·s)]
  const double T   = sensor_params_.temperature;
  auto carrier_e   = tracking::digitization::getCarrier(-1);
  auto carrier_h   = tracking::digitization::getCarrier(1);
  const double mu_e = carrier_e.mu0(T) * 1.0e-4;  // m²/(V·s)
  const double mu_h = carrier_h.mu0(T) * 1.0e-4;

  auto bfield_cache = bField()->makeCache(magneticFieldContext());

  for (const auto& [layer_id, surface] : geometry().layer_surface_map_) {
    const Acts::Vector3 center_mm = surface->center(geometryContext());
    const auto b_result = bField()->getField(center_mm, bfield_cache);
    if (!b_result.ok()) continue;

    // B in Tesla (ACTS field providers return values in Acts internal units)
    const Acts::Vector3 b_T = b_result.value() / Acts::UnitConstants::T;

    // Sensor W-normal = 3rd column of the rotation matrix
    const Acts::Vector3 w_hat =
        surface->transform(geometryContext()).rotation().col(2);

    const double Bw = b_T.dot(w_hat);  // [T]

    // tan(θ_L) = charge_sign · μ · Bw
    // electrons: charge = −1, holes: charge = +1
    const double tan_e = -mu_e * Bw;
    const double tan_h = +mu_h * Bw;

    lorentz_tan_cache_[layer_id] = {tan_e, tan_h};

    ldmx_log(debug) << "Lorentz cache: layer=" << layer_id
                    << "  Bw=" << Bw << " T"
                    << "  tan_e=" << tan_e
                    << "  tan_h=" << tan_h;
  }

  ldmx_log(info) << "Lorentz tangents computed for "
                 << lorentz_tan_cache_.size() << " layers from field map "
                 << (field_map_.empty() ? geometry().fieldMapFile() : field_map_);
}

void DigitizationProcessor::onNewRun(const ldmx::RunHeader& runHeader) {
  const auto& rseed = getCondition<framework::RandomNumberSeedService>(
      framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
  generator_.seed(rseed.getSeed("Tracking::DigitizationProcessor"));
}

void DigitizationProcessor::produce(framework::Event& event) {
  ldmx_log(trace) << " Getting the tracking geometry:" << geometry().getTG();

  const std::vector<ldmx::SimTrackerHit> sim_hits =
      event.getCollection<ldmx::SimTrackerHit>(hit_collection_,
                                               tracker_hit_passname_);

  std::vector<ldmx::SimTrackerHit>  merged_hits;
  std::vector<ldmx::Measurement>    measurements;
  std::vector<ldmx::RawSiStripHit>  raw_hits;

  const bool save_raw = use_charge_digitization_ && !out_raw_collection_.empty();
  auto* raw_ptr = save_raw ? &raw_hits : nullptr;

  if (merge_hits_) {
    mergeSimHits(sim_hits, merged_hits);
    measurements = digitizeHits(merged_hits, raw_ptr);
  } else {
    measurements = digitizeHits(sim_hits, raw_ptr);
  }

  event.add(out_collection_, measurements);
  if (save_raw) {
    event.add(out_raw_collection_, raw_hits);
  }
}

// ---------------------------------------------------------------------------
// mergeHits / mergeSimHits
// ---------------------------------------------------------------------------

bool DigitizationProcessor::mergeHits(
    const std::vector<ldmx::SimTrackerHit>& sihits,
    std::vector<ldmx::SimTrackerHit>& mergedHits) {
  if (sihits.size() < 1) return false;

  if (sihits.size() == 1) {
    mergedHits.push_back(sihits[0]);
    return true;
  }

  ldmx::SimTrackerHit merged_hit;
  merged_hit.setLayerID(sihits[0].getLayerID());
  merged_hit.setModuleID(sihits[0].getModuleID());
  merged_hit.setID(sihits[0].getID());
  merged_hit.setTrackID(sihits[0].getTrackID());

  double x{0}, y{0}, z{0}, px{0}, py{0}, pz{0};
  double t{0}, e{0}, edep{0}, path{0};
  int pdg_id = sihits[0].getPdgID();

  for (auto hit : sihits) {
    double edep_hit = hit.getEdep();
    edep += edep_hit;
    e    += hit.getEnergy();
    t    += edep_hit * hit.getTime();
    x    += edep_hit * hit.getPosition()[0];
    y    += edep_hit * hit.getPosition()[1];
    z    += edep_hit * hit.getPosition()[2];
    px   += edep_hit * hit.getMomentum()[0];
    py   += edep_hit * hit.getMomentum()[1];
    pz   += edep_hit * hit.getMomentum()[2];
    path += edep_hit * hit.getPathLength();

    if (hit.getPdgID() != pdg_id) {
      ldmx_log(error)
          << "ERROR:: Found hits with compatible sensorID and track_id "
             "but different PDGID";
      ldmx_log(error) << "TRACKID ==" << hit.getTrackID() << " vs "
                      << sihits[0].getTrackID();
      ldmx_log(error) << "PDGID== " << hit.getPdgID() << " vs " << pdg_id;
      return false;
    }
  }

  merged_hit.setTime(t / edep);
  merged_hit.setPosition(x / edep, y / edep, z / edep);
  merged_hit.setMomentum(px / edep, py / edep, pz / edep);
  merged_hit.setPathLength(path / edep);
  merged_hit.setEnergy(e);
  merged_hit.setEdep(edep);
  merged_hit.setPdgID(pdg_id);

  mergedHits.push_back(merged_hit);
  return true;
}

bool DigitizationProcessor::mergeSimHits(
    const std::vector<ldmx::SimTrackerHit>& sim_hits,
    std::vector<ldmx::SimTrackerHit>& merged_hits) {
  // Key: [sensor_id][track_id] → list of hits to merge
  std::map<int, std::map<int, std::vector<ldmx::SimTrackerHit>>> hitmap;

  for (const auto& hit : sim_hits) {
    unsigned int index   = tracking::sim::utils::getSensorID(hit);
    unsigned int trackid = hit.getTrackID();
    hitmap[index][trackid].push_back(hit);

    ldmx_log(trace) << "hitmap being filled, size::[" << index << "]["
                    << trackid << "] size " << hitmap[index][trackid].size();
  }

  typedef std::map<int,
                   std::map<int, std::vector<ldmx::SimTrackerHit>>>::iterator
      hitmap_it1;
  typedef std::map<int, std::vector<ldmx::SimTrackerHit>>::iterator hitmap_it2;

  for (hitmap_it1 it = hitmap.begin(); it != hitmap.end(); it++) {
    for (hitmap_it2 it2 = it->second.begin(); it2 != it->second.end(); it2++) {
      mergeHits(it2->second, merged_hits);
    }
  }

  ldmx_log(debug) << "Sim_hits Size = " << sim_hits.size()
                  << " Merged_hits Size = " << merged_hits.size();

  for (const auto& hit  : sim_hits)    { ldmx_log(trace) << hit; }
  for (const auto& mhit : merged_hits) { ldmx_log(trace) << mhit; }

  return true;
}

// ---------------------------------------------------------------------------
// digitizeHits — Mode 0 (smearing) and Mode 1 (charge digitization)
// ---------------------------------------------------------------------------

std::vector<ldmx::Measurement> DigitizationProcessor::digitizeHits(
    const std::vector<ldmx::SimTrackerHit>& sim_hits,
    std::vector<ldmx::RawSiStripHit>* raw_hits) {
  ldmx_log(debug) << "Found: " << sim_hits.size() << " sim hits in '"
                  << hit_collection_ << "' with passname '"
                  << tracker_hit_passname_ << "'";

  std::vector<ldmx::Measurement> measurements;

  for (auto& sim_hit : sim_hits) {
    // Energy deposition cut
    if (sim_hit.getEdep() <= min_e_dep_) continue;
    if (track_id_ > 0 && sim_hit.getTrackID() != track_id_) continue;

    ldmx::Measurement measurement(sim_hit);

    // Sensor identification
    auto layer_id = tracking::sim::utils::getSensorID(sim_hit);
    measurement.setLayerID(layer_id);

    auto hit_surface{geometry().getSurface(layer_id)};
    if (!hit_surface) continue;

    ldmx_log(trace) << "Local to global\n"
                    << hit_surface->transform(geometryContext()).rotation()
                    << "\n"
                    << hit_surface->transform(geometryContext()).translation();

    // -----------------------------------------------------------------------
    // Project global hit position onto the surface (2D local coords)
    // -----------------------------------------------------------------------
    Acts::Vector3 dummy_momentum;
    Acts::Vector2 local_pos_2d;

    // TODO: clarify / derive the 0.320 mm surface tolerance from the geometry
    constexpr double surface_thickness = 0.320 * Acts::UnitConstants::mm;

    Acts::Vector3 global_pos(measurement.getGlobalPosition()[0],
                             measurement.getGlobalPosition()[1],
                             measurement.getGlobalPosition()[2]);

    try {
      local_pos_2d =
          hit_surface
              ->globalToLocal(geometryContext(), global_pos, dummy_momentum,
                              surface_thickness)
              .value();
    } catch (const std::exception& e) {
      ldmx_log(warn) << "hit not on surface... Skipping.";
      continue;
    }

    // Store the projected truth U before any smearing or charge digitization.
    measurement.setTruthU(static_cast<float>(local_pos_2d[0]));

    // -----------------------------------------------------------------------
    // Mode 1: realistic charge digitization
    // -----------------------------------------------------------------------
    if (use_charge_digitization_) {
      // Read sensor thickness from the geometry.
      const auto* det_el = hit_surface->associatedDetectorElement();
      if (!det_el) {
        ldmx_log(warn) << "No detector element for layer_id=" << layer_id
                       << " — skipping hit";
        continue;
      }
      const double thickness = det_el->thickness();
      strip_digitizer_->setThickness(thickness);

      // Build the full 3D local position and direction for charge simulation.
      const Acts::Transform3 surf_transform =
          hit_surface->transform(geometryContext());

      // 3D local position: apply the inverse surface transform to the global
      // hit position so that we know the depth (W) coordinate.
      const Acts::Vector3 local_pos_3d =
          surf_transform.inverse() * global_pos;

      // 3D local direction: rotate the global unit momentum into local frame.
      Acts::Vector3 global_mom(sim_hit.getMomentum()[0],
                               sim_hit.getMomentum()[1],
                               sim_hit.getMomentum()[2]);
      const double mom_mag = global_mom.norm();

      Acts::Vector3 local_dir_3d;
      if (mom_mag > 0.0) {
        local_dir_3d =
            surf_transform.rotation().transpose() * (global_mom / mom_mag);
      } else {
        // Degenerate case: treat as normal incidence
        local_dir_3d = Acts::Vector3(0.0, 0.0, 1.0);
      }

      // Path length through the sensor; fall back to thickness / |cos θ|
      // if the stored value is not set.
      double path_length = sim_hit.getPathLength();
      if (path_length <= 0.0) {
        const double cos_theta = std::abs(local_dir_3d[2]);
        path_length = (cos_theta > 1e-3) ? thickness / cos_theta : thickness;
      }

      // Apply per-layer Lorentz tangents from the B-field cache (if available).
      auto lorentz_it = lorentz_tan_cache_.find(layer_id);
      if (lorentz_it != lorentz_tan_cache_.end()) {
        strip_digitizer_->mutableParams().electron_lorentz_tangent =
            lorentz_it->second.first;
        strip_digitizer_->mutableParams().hole_lorentz_tangent =
            lorentz_it->second.second;
      }

      // Compute charge deposited on each strip
      auto strip_charges = strip_digitizer_->computeStripCharges(
          sim_hit.getEdep(), local_pos_3d, local_dir_3d, path_length);

      ldmx_log(trace) << "Charge digi: " << strip_charges.size()
                      << " strips before threshold";

      // Add noise and apply threshold
      strip_digitizer_->applyNoiseAndThreshold(strip_charges);

      ldmx_log(trace) << "Charge digi: " << strip_charges.size()
                      << " strips after threshold";

      if (strip_charges.empty()) {
        ldmx_log(debug) << "Hit suppressed by threshold after noise addition.";
        continue;
      }

      // Cluster to get the U measurement
      auto [centroid_u, resolution_u] =
          strip_digitizer_->clusterToPosition(strip_charges);

      // Keep V from the simple 2D projection (strips give no V information)
      const double meas_v  = local_pos_2d[1];
      const double sigma_v_charge = sigma_v_ > 0.0 ? sigma_v_ : 1.0;  // [mm]

      measurement.setLocalPosition(static_cast<float>(centroid_u),
                                   static_cast<float>(meas_v));
      measurement.setLocalCovariance(static_cast<float>(resolution_u * resolution_u),
                                     static_cast<float>(sigma_v_charge * sigma_v_charge));

      // Update the global position from the new local U (keep V, Z unchanged)
      auto transf_global =
          hit_surface->localToGlobal(geometryContext(),
                                     Acts::Vector2(centroid_u, meas_v),
                                     dummy_momentum);
      measurement.setGlobalPosition(measurement.getGlobalPosition()[0],
                                    transf_global(1),
                                    transf_global(2));

      // Produce RawSiStripHit objects (N_SAMPLES shaped ADC samples per strip).
      if (raw_hits && pulse_shape_) {
        const int    adc_max  = (1 << tracking::digitization::ADC_BITS) - 1;
        const double hit_time_ns = sim_hit.getTime();  // absolute hit arrival time [ns]
        const long   hit_time    = static_cast<long>(hit_time_ns);

        for (const auto& [strip_idx, charge] : strip_charges) {
          // Peak ADC value (before shaping) from the collected charge.
          const double peak_adc = charge / tracking::digitization::ADC_ELECTRONS_PER_COUNT;

          std::vector<short> samples(tracking::digitization::N_SAMPLES);
          for (int isamp = 0; isamp < tracking::digitization::N_SAMPLES; ++isamp) {
            // t_sample is the absolute time of this ADC clock tick.
            // The pulse shape argument is (t_sample - T_hit): time since
            // hit arrival, so the fitter can recover T_hit as result.t0.
            const double t_sample = tracking::digitization::T0_OFFSET_NS
                                    + isamp * tracking::digitization::SAMPLING_INTERVAL_NS;
            const double val = tracking::digitization::ADC_PEDESTAL
                               + peak_adc * pulse_shape_->eval(t_sample - hit_time_ns);
            samples[isamp] = static_cast<short>(
                std::clamp(static_cast<int>(std::round(val)), 0, adc_max));
          }

          raw_hits->emplace_back(layer_id, strip_idx, std::move(samples),
                                 hit_time, sim_hit.getTrackID(),
                                 sim_hit.getPdgID(), sim_hit.getID(),
                                 sim_hit.getEdep());
        }
      }

      measurements.push_back(measurement);

    // -----------------------------------------------------------------------
    // Mode 0: simple Gaussian smearing
    // -----------------------------------------------------------------------
    } else {
      if (do_smearing_) {
        float smear_factor{(*normal_)(generator_)};
        local_pos_2d[0] += smear_factor * sigma_u_;
        smear_factor = (*normal_)(generator_);
        local_pos_2d[1] += smear_factor * sigma_v_;

        measurement.setLocalCovariance(sigma_u_ * sigma_u_,
                                       sigma_v_ * sigma_v_);

        auto transf_global_pos{hit_surface->localToGlobal(
            geometryContext(), local_pos_2d, dummy_momentum)};
        measurement.setGlobalPosition(measurement.getGlobalPosition()[0],
                                      transf_global_pos(1),
                                      transf_global_pos(2));
      }

      measurement.setLocalPosition(local_pos_2d(0), local_pos_2d(1));
      measurements.push_back(measurement);
    }
  }  // loop over sim hits

  return measurements;
}  // digitizeHits

}  // namespace tracking::reco

DECLARE_PRODUCER(tracking::reco::DigitizationProcessor)
