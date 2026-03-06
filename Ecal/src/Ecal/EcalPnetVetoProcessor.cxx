#include "Ecal/EcalPnetVetoProcessor.h"

namespace ecal {

const std::vector<std::string> EcalPnetVetoProcessor::INPUT_NAMES{"points",
                                                                  "features"};
const std::vector<unsigned int> EcalPnetVetoProcessor::INPUT_SIZES{
    N_COORDINATE_DIM * MAX_NUM_HITS, N_FEATURE_DIM* MAX_NUM_HITS};

EcalPnetVetoProcessor::EcalPnetVetoProcessor(const std::string& name,
                                             framework::Process& process)
    : Producer(name, process) {
  for (const auto& s : INPUT_SIZES) {
    data_.emplace_back(s, 0);
  }
}

void EcalPnetVetoProcessor::configure(
    framework::config::Parameters& parameters) {
  disc_cut_ = parameters.get<double>("disc_cut");
  rt_ = std::make_unique<ldmx::ort::ONNXRuntime>(
      parameters.get<std::string>("model_path"));

  // Set the collection name as defined in the configuration
  collection_name_ = parameters.get<std::string>("collection_name");

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

  // Get the collection of digitized Ecal hits_ from the event.
  const auto ecal_rec_hits = event.getCollection<ldmx::EcalHit>(
      rec_coll_name_, ecal_rec_hits_passname_);
  auto nhits = std::count_if(
      ecal_rec_hits.begin(), ecal_rec_hits.end(),
      [](const ldmx::EcalHit& hit) { return hit.getEnergy() > 0; });

  // check number of hits_
  ldmx_log(debug) << "nhits = " << nhits << " MAX_NUM_HITS = " << MAX_NUM_HITS;
  if (nhits < MAX_NUM_HITS) {
    std::array<double, 3> etraj = {-999., -999., -999.};
    std::array<double, 3> enorm = {-999., -999., -999.};
    // Compute electron trajectory}
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
      // If we found an electron hit at the scoring plane
      if (electron_hit) {
        // Get electron hit position/momentum at Ecal surface
        ldmx_log(debug) << "Electron Found in the Ecal SP!";
        auto pos = electron_hit->getPosition();
        auto mom = electron_hit->getMomentum();
        ldmx_log(debug) << "ECAL SP pos_=(" << pos[0] << "," << pos[1] << ","
                        << pos[2] << ")";
        ldmx_log(debug) << "ECAL SP mom=(" << mom[0] << "," << mom[1] << ","
                        << mom[2] << ")";
        etraj = {pos[0], pos[1], pos[2]};
        double pz = mom[2];
        if (pz != 0) {
          // z_-normalized momentum
          enorm = {mom[0] / pz, mom[1] / pz, 1.0};
        }
      }
    } else if (recoil_from_tracking_) {
      // Use tracking to get electron hit position/momentum at Ecal surface
      auto recoil_tracks{event.getCollection<ldmx::Track>(track_collection_,
                                                             track_pass_name_)};
      ldmx::TrackStateType ts_at_ecal = ldmx::AtECAL;
      auto recoil_track_states_ecal =
          ecal::trackProp(recoil_tracks, ts_at_ecal, "ecal");
      if (!recoil_track_states_ecal.empty()) {
        std::array<double, 3> pos = {recoil_track_states_ecal[0],
                                     recoil_track_states_ecal[1],
                                     recoil_track_states_ecal[2]};
        std::array<double, 3> mom = {(recoil_track_states_ecal[3]),
                                     (recoil_track_states_ecal[4]),
                                     (recoil_track_states_ecal[5])};
        ldmx_log(debug) << "Electron track pos_=(" << pos[0] << "," << pos[1]
                        << "," << pos[2] << ")";
        ldmx_log(debug) << "Electron track mom=(" << mom[0] << "," << mom[1]
                        << "," << mom[2] << ")";
        etraj = pos;
        double pz = mom[2];
        if (pz != 0) {
          enorm = {mom[0] / pz, mom[1] / pz, 1.0};
        }
      } else {
        ldmx_log(info) << "  No recoil track at ECAL";
      }
    } else {
      ldmx_log(fatal) << "  No electron hit at scoring plane or no tracking";
    }

    // make inputs
    makeInputs(ecal_geometry, ecal_rec_hits, etraj, enorm);
    // run the DNN
    auto logits = rt_->run(INPUT_NAMES, data_)[0];
    // make a log softmax of the logits then transform back
    // to a probability with an exponential
    auto prob = std::exp((logSoftmax(logits)[1]));
    result.setDiscValue(prob);
  } else {
    result.setDiscValue(-99);
  }

  ldmx_log(debug) << "ParticleNet disc value = " << result.getDisc();

  result.setVetoResult(result.getDisc() > disc_cut_);

  // If the event passes the veto, keep it. Otherwise, drop the event.
  if (result.passesVeto()) {
    setStorageHint(framework::HINT_SHOULD_KEEP);
  } else {
    setStorageHint(framework::HINT_SHOULD_DROP);
  }

  event.add(collection_name_, result);
}

void EcalPnetVetoProcessor::makeInputs(
    const ldmx::EcalGeometry& geom,
    const std::vector<ldmx::EcalHit>& ecal_rec_hits,
    std::array<double, 3> etraj, std::array<double, 3> enorm) {
  // clear data
  for (auto& v : data_) {
    std::fill(v.begin(), v.end(), 0);
  }

  // Loop on the rechits
  unsigned idx = 0;
  for (const auto& hit : ecal_rec_hits) {
    if (hit.getEnergy() <= 0) {
      ldmx_log(warn)
          << "Hit with zero energy found, should not happen, skipping it.";
      continue;
    }
    ldmx::EcalID id(hit.getID());
    auto [hit_x, hit_y, hit_z] = geom.getPosition(id);

    double delta_z = hit_z - etraj[2];
    double etraj_x = etraj[0] + enorm[0] * delta_z;
    double etraj_y = etraj[1] + enorm[1] * delta_z;
    data_[0].at(COORDINATE_X_OFFSET + idx) = hit_x - etraj_x;
    data_[0].at(COORDINATE_Y_OFFSET + idx) = hit_y - etraj_y;
    data_[0].at(COORDINATE_Z_OFFSET + idx) = hit_z;

    data_[1].at(FEATURE_X_OFFSET + idx) = hit_x - etraj_x;
    data_[1].at(FEATURE_Y_OFFSET + idx) = hit_y - etraj_y;
    data_[1].at(FEATURE_Z_OFFSET + idx) = hit_z;
    data_[1].at(FEATURE_LAYER_ID_OFFSET + idx) = id.layer();
    data_[1].at(FEATURE_ENERGY_OFFSET + idx) = std::log(hit.getEnergy());

    ++idx;
  }

  std::stringstream ss;
  for (unsigned iname = 0; iname < INPUT_NAMES.size(); ++iname) {
    ss << "=== " << INPUT_NAMES[iname] << " ===";
    for (unsigned i = 0; i < INPUT_SIZES[iname]; ++i) {
      ss << data_[iname].at(i) << ", ";
      if ((i + 1) % MAX_NUM_HITS == 0) {
        ss << "\n\n";
      }
    }
  }
  ldmx_log(trace) << ss.str();
}  // end of make inputs

std::vector<float> EcalPnetVetoProcessor::logSoftmax(
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
