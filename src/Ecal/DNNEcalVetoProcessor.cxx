#include "Ecal/DNNEcalVetoProcessor.h"

// LDMX
#include "Ecal/Event/EcalHit.h"

#include <algorithm>

namespace ecal {

const std::vector<std::string> DNNEcalVetoProcessor::input_names_{"coordinates",
                                                                  "features"};
const std::vector<unsigned int> DNNEcalVetoProcessor::input_sizes_{
    n_coordinate_dim_ * max_num_hits_, n_feature_dim_* max_num_hits_};

DNNEcalVetoProcessor::DNNEcalVetoProcessor(const std::string& name,
                                           framework::Process& process)
    : Producer(name, process) {
  for (const auto& s : input_sizes_) {
    data_.emplace_back(s, 0);
  }
}

void DNNEcalVetoProcessor::configure(
    framework::config::Parameters& parameters) {
  disc_cut_ = parameters.getParameter<double>("disc_cut");
  rt_ = std::make_unique<ldmx::Ort::ONNXRuntime>(
      parameters.getParameter<std::string>("model_path"));

  // debug mode
  debug_ = parameters.getParameter<bool>("debug");

  // Set the collection name as defined in the configuration
  collectionName_ = parameters.getParameter<std::string>("collection_name");
}

void DNNEcalVetoProcessor::produce(framework::Event& event) {
  ldmx::EcalVetoResult result;

  // Get the Ecal Geometry
  const auto& ecal_geometry = getCondition<ldmx::EcalGeometry>(
      ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  // Get the collection of digitized Ecal hits from the event.
  const auto ecalRecHits = event.getCollection<ldmx::EcalHit>("EcalRecHits");
  auto nhits = std::count_if(
      ecalRecHits.begin(), ecalRecHits.end(),
      [](const ldmx::EcalHit& hit) { return hit.getEnergy() > 0; });

  if (nhits < max_num_hits_) {
    // make inputs
    make_inputs(ecal_geometry, ecalRecHits);
    // run the DNN
    auto outputs = rt_->run(input_names_, data_)[0];
    result.setDiscValue(outputs.at(1));
  } else {
    result.setDiscValue(-99);
  }

  if (debug_) {
    std::cout << "... disc_val = " << result.getDisc() << std::endl;
  }

  result.setVetoResult(result.getDisc() > disc_cut_);

  // If the event passes the veto, keep it. Otherwise, drop the event.
  if (result.passesVeto()) {
    setStorageHint(framework::hint_shouldKeep);
  } else {
    setStorageHint(framework::hint_shouldDrop);
  }

  event.add(collectionName_, result);
}

void DNNEcalVetoProcessor::make_inputs(
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
    auto [x,y,z] = geom.getPosition(id);

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

  if (debug_) {
    for (unsigned iname = 0; iname < input_names_.size(); ++iname) {
      std::cout << "=== " << input_names_[iname] << " ===" << std::endl;
      for (unsigned i = 0; i < input_sizes_[iname]; ++i) {
        std::cout << data_[iname].at(i) << ", ";
        if ((i + 1) % max_num_hits_ == 0) {
          std::cout << std::endl;
        }
      }
    }
  }  // debug
}

}  // namespace ecal

DECLARE_PRODUCER_NS(ecal, DNNEcalVetoProcessor);
