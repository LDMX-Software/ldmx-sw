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
  disc_cut_ = parameters.getParameter<double>("disc_cut");
  rt_ = std::make_unique<ldmx::Ort::ONNXRuntime>(
      parameters.getParameter<std::string>("model_path"));

  // max number of hits that this veto looks at
  // max_num_hits_ = parameters.getParameter<int>("max_num_hits");

  // Set the collection name as defined in the configuration
  collectionName_ = parameters.getParameter<std::string>("collection_name");

  ecal_rec_hits_passname_ =
      parameters.getParameter<std::string>("ecal_rec_hits_passname");
}

void EcalPnetVetoProcessor::produce(framework::Event& event) {
  ldmx::EcalVetoResult result;

  // Get the Ecal Geometry
  const auto& ecal_geometry = getCondition<ldmx::EcalGeometry>(
      ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  // Get the collection of digitized Ecal hits from the event.
  const auto ecalRecHits = event.getCollection<ldmx::EcalHit>(
      "EcalRecHits", ecal_rec_hits_passname_);
  auto nhits = std::count_if(
      ecalRecHits.begin(), ecalRecHits.end(),
      [](const ldmx::EcalHit& hit) { return hit.getEnergy() > 0; });

  ldmx_log(trace) << "nhits = " << nhits
                  << " max_num_hits_ = " << max_num_hits_;

  if (nhits < max_num_hits_) {
    // make inputs
    make_inputs(ecal_geometry, ecalRecHits);
    // run the DNN
    auto logits = rt_->run(input_names_, data_)[0];
    // make a log softmax of the logits then transform back
    // to a probability with an exponential
    auto prob = std::exp((log_softmax(logits)[1]));
    result.setDiscValue(prob);
  } else {
    result.setDiscValue(-99);
  }

  ldmx_log(info) << "ParticleNet disc valu = " << result.getDisc();

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
    const std::vector<ldmx::EcalHit>& ecalRecHits) {
  // clear data
  for (auto& v : data_) {
    std::fill(v.begin(), v.end(), 0);
  }

  unsigned idx = 0;
  for (const auto& hit : ecalRecHits) {
    if (hit.getEnergy() <= 0) continue;
    ldmx::EcalID id(hit.getID());
    auto [x, y, z] = geom.getPosition(id);

    data_[0].at(coordinate_x_offset_ + idx) = x;
    data_[0].at(coordinate_y_offset_ + idx) = y;
    data_[0].at(coordinate_z_offset_ + idx) = z;

    data_[1].at(feature_x_offset_ + idx) = x;
    data_[1].at(feature_y_offset_ + idx) = y;
    data_[1].at(feature_z_offset_ + idx) = z;
    data_[1].at(feature_layerid_offset_ + idx) = id.layer();
    data_[1].at(feature_energy_offset_ + idx) = std::log(hit.getEnergy());

    ++idx;
  }

  for (unsigned iname = 0; iname < input_names_.size(); ++iname) {
    ldmx_log(trace) << "=== " << input_names_[iname] << " ===";
    for (unsigned i = 0; i < input_sizes_[iname]; ++i) {
      ldmx_log(trace) << data_[iname].at(i) << ", ";
      if ((i + 1) % max_num_hits_ == 0) {
        ldmx_log(trace) << "\n\n";
      }
    }
  }
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
