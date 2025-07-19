#include "Ecal/EcalPnetVetoProcessor.h"

namespace ecal {

const std::vector<std::string> EcalPnetVetoProcessor::input_names_{"points",
                                                                   "features"};
const std::vector<unsigned int> EcalPnetVetoProcessor::input_sizes_{
    n_coordinate_dim_ * max_num_hits_, n_feature_dim_* max_num_hits_};

EcalPnetVetoProcessor::EcalPnetVetoProcessor(const std::string& name,
                                             framework::Process& process)
    : Producer(name, process) {
  for (const auto& s : input_sizes_) {
    data_.emplace_back(s, 0);
  }
}

void EcalPnetVetoProcessor::configure(
    framework::config::Parameters& parameters) {
  disc_cut_ = parameters.get<double>("disc_cut");
  rt_ = std::make_unique<ldmx::Ort::ONNXRuntime>(
      parameters.get<std::string>("model_path"));

  // Set the collection name as defined in the configuration
  collectionName_ = parameters.get<std::string>("collection_name");

  rec_coll_name_ = parameters.get<std::string>("rec_coll_name");
  ecal_rec_hits_passname_ =
      parameters.get<std::string>("ecal_rec_hits_passname");
  ecal_sp_hits_passname_ = parameters.get<std::string>("ecal_sp_hits_passname");
  track_pass_name_ = parameters.get<std::string>("track_pass_name", "");
  track_collection_ = parameters.get<std::string>("track_collection");
  recoil_from_tracking_ = parameters.get<bool>("recoil_from_tracking");
}

void EcalPnetVetoProcessor::produce(framework::Event& event) {
  ldmx::EcalVetoResult result;
  //  Get the Ecal Geometry
  const auto& ecal_geometry = getCondition<ldmx::EcalGeometry>(
      ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  // Get the collection of digitized Ecal hits from the event.
  const auto ecal_rec_hits = event.getCollection<ldmx::EcalHit>(
      rec_coll_name_, ecal_rec_hits_passname_);
  auto nhits = std::count_if(
      ecal_rec_hits.begin(), ecal_rec_hits.end(),
      [](const ldmx::EcalHit& hit) { return hit.getEnergy() > 0; });

  // check number of hits
  ldmx_log(trace) << "nhits = " << nhits
                  << " max_num_hits_ = " << max_num_hits_;
  if (nhits < max_num_hits_) {
    // make inputs
    make_inputs(ecal_geometry, ecal_rec_hits, event);
    // run the DNN
    auto logits = rt_->run(input_names_, data_)[0];
    // make a log softmax of the logits then transform back
    // to a probability with an exponential
    auto prob = std::exp((log_softmax(logits)[1]));
    result.setDiscValue(prob);
  } else {
    result.setDiscValue(-99);
  }

  ldmx_log(info) << "ParticleNet disc value = " << result.getDisc();

  result.setVetoResult(result.getDisc() > disc_cut_);

  // If the event passes the veto, keep it. Otherwise, drop the event.
  if (result.passesVeto()) {
    setStorageHint(framework::hint_shouldKeep);
  } else {
    setStorageHint(framework::hint_shouldDrop);
  }

  event.add(collectionName_, result);
}

void EcalPnetVetoProcessor::make_inputs(
    const ldmx::EcalGeometry& geom,
    const std::vector<ldmx::EcalHit>& ecal_rec_hits,
    const framework::Event& event) {
  // Compute electron trajectory
  std::array<double, 3> etraj_sp = {-999., -999., -999.};
  std::array<double, 3> enorm_sp = {-999., -999., -999.};
  const ldmx::SimTrackerHit* electron_hit = nullptr;
  // Use Scoring Plane or Tracking
  if (!recoil_from_tracking_ &&
      event.exists("EcalScoringPlaneHits", ecal_sp_hits_passname_)) {
    auto const& ecal_sp_hits = event.getCollection<ldmx::SimTrackerHit>(
        "EcalScoringPlaneHits", ecal_sp_hits_passname_);
    double electron_pz_max = -1.0;
    for (auto const& hit : ecal_sp_hits) {
      // Look at the electron only
      if (hit.getPdgID() != 11) continue;
      double electron_z = hit.getPosition()[2];
      // Look at the SP in front of the ECAL
      if (electron_z <= 239.0 || electron_z >= 240.0) continue;
      double electron_pz = hit.getMomentum()[2];
      // Find the highest pz electron
      if (electron_pz > electron_pz_max) {
        electron_pz_max = electron_pz;
        electron_hit = &hit;
      }
    }
    if (electron_hit) {
      // Get electron hit position/momentum at Ecal surface
      ldmx_log(trace) << "Electron Found!";
      auto pos = electron_hit->getPosition();
      auto mom = electron_hit->getMomentum();
      ldmx_log(info) << "SPpos=(" << pos[0] << "," << pos[1] << "," << pos[2]
                     << ")";
      ldmx_log(info) << "SPmom=(" << mom[0] << "," << mom[1] << "," << mom[2]
                     << ")";
      etraj_sp = {pos[0], pos[1], pos[2]};
      double pz = mom[2];
      if (pz != 0) {
        // z-normalized momentum
        enorm_sp = {mom[0] / pz, mom[1] / pz, 1.0};
      }
    }
  } else {
    // Use tracking to get electron hit position/momentum at Ecal surface
    auto recoil_tracks{
        event.getCollection<ldmx::Track>(track_collection_, track_pass_name_)};
    ldmx::TrackStateType ts_type = ldmx::TrackStateType::AtECAL;
    auto recoil_track_states_ecal =
        ecal::TrackPropagator::trackProp(recoil_tracks, ts_type, "ecal");
    if (!recoil_track_states_ecal.empty()) {
      std::array<double, 3> pos = {recoil_track_states_ecal[0],
                                   recoil_track_states_ecal[1],
                                   recoil_track_states_ecal[2]};
      std::array<double, 3> mom = {(recoil_track_states_ecal[3]),
                                   (recoil_track_states_ecal[4]),
                                   (recoil_track_states_ecal[5])};
      ldmx_log(info) << "Electron track pos=(" << pos[0] << "," << pos[1] << ","
                     << pos[2] << ")";
      ldmx_log(info) << "Electron track mom=(" << mom[0] << "," << mom[1] << ","
                     << mom[2] << ")";
      etraj_sp = pos;
      double pz = mom[2];
      if (pz != 0) {
        enorm_sp = {mom[0] / pz, mom[1] / pz, 1.0};
      }
    } else {
      ldmx_log(info) << "  No recoil track at ECAL";
    }
  }

  // clear data
  for (auto& v : data_) {
    std::fill(v.begin(), v.end(), 0);
  }

  // Loop on the rechits
  unsigned idx = 0;
  for (const auto& hit : ecal_rec_hits) {
    if (hit.getEnergy() <= 0) continue;
    ldmx::EcalID id(hit.getID());
    auto [x, y, z] = geom.getPosition(id);

    // Compute relative position
    double etraj_x = -999.;
    double etraj_y = -999.;
    if (electron_hit) {
      double delta_z = z - etraj_sp[2];
      etraj_x = etraj_sp[0] + enorm_sp[0] * delta_z;
      etraj_y = etraj_sp[1] + enorm_sp[1] * delta_z;
    }
    data_[0].at(coordinate_x_offset_ + idx) = x - etraj_x;
    data_[0].at(coordinate_y_offset_ + idx) = y - etraj_y;
    data_[0].at(coordinate_z_offset_ + idx) = z;

    data_[1].at(feature_x_offset_ + idx) = x - etraj_x;
    data_[1].at(feature_y_offset_ + idx) = y - etraj_y;
    data_[1].at(feature_z_offset_ + idx) = z;
    data_[1].at(feature_layerid_offset_ + idx) = id.layer();
    data_[1].at(feature_energy_offset_ + idx) = std::log(hit.getEnergy());

    ++idx;
  }

  std::stringstream ss;
  for (unsigned iname = 0; iname < input_names_.size(); ++iname) {
    ss << "=== " << input_names_[iname] << " ===";
    for (unsigned i = 0; i < input_sizes_[iname]; ++i) {
      ss << data_[iname].at(i) << ", ";
      if ((i + 1) % max_num_hits_ == 0) {
        ss << "\n\n";
      }
    }
  }
  ldmx_log(trace) << ss.str();
}  // end of make inputs

std::vector<float> EcalPnetVetoProcessor::log_softmax(
    const std::vector<float>& logits) {
  // Find max for numerical stability
  auto max_val = *std::max_element(logits.begin(), logits.end());

  // Compute shifted exponentials and their sum
  std::vector<float> exp_vals(logits.size());
  for (size_t i = 0; i < logits.size(); ++i) {
    exp_vals[i] = std::exp(logits[i] - max_val);
  }

  float sum_exp = std::accumulate(exp_vals.begin(), exp_vals.end(), 0.0);
  float log_sum_exp = max_val + std::log(sum_exp);

  // Compute log_softmax
  std::vector<float> result(logits.size());
  for (size_t i = 0; i < logits.size(); ++i) {
    result[i] = logits[i] - log_sum_exp;
  }

  return result;
}

}  // namespace ecal

DECLARE_PRODUCER(ecal::EcalPnetVetoProcessor);