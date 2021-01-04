#include "Ecal/DNNEcalVetoProcessor.h"

// LDMX
#include "Ecal/Event/EcalHit.h"

#include <algorithm>

namespace ldmx {

const std::vector<std::string> DNNEcalVetoProcessor::input_names_{"coordinates",
                                                                  "features"};
const std::vector<unsigned int> DNNEcalVetoProcessor::input_sizes_{
    n_coordinate_dim_ * max_num_hits_, n_feature_dim_* max_num_hits_};

DNNEcalVetoProcessor::DNNEcalVetoProcessor(const std::string& name,
                                           Process& process)
    : Producer(name, process) {
  for (const auto& s : input_sizes_) {
    data_.emplace_back(s, 0);
  }
}

void DNNEcalVetoProcessor::configure(Parameters& parameters) {
  disc_cut_ = parameters.getParameter<double>("disc_cut");
  rt_ = std::make_unique<Ort::ONNXRuntime>(
      parameters.getParameter<std::string>("model_path"));

  // debug mode
  debug_ = parameters.getParameter<bool>("debug");

  // Set the collection name as defined in the configuration
  collectionName_ = parameters.getParameter<std::string>("collection_name");
}

void DNNEcalVetoProcessor::produce(Event& event) {
  EcalVetoResult result;

  // Get the Ecal Geometry
  const EcalHexReadout& hexReadout =
      getCondition<EcalHexReadout>(EcalHexReadout::CONDITIONS_OBJECT_NAME);

  // Get the collection of digitized Ecal hits from the event.
  const auto ecalRecHits = event.getCollection<EcalHit>("EcalRecHits");
  auto nhits =
      std::count_if(ecalRecHits.begin(), ecalRecHits.end(),
                    [](const EcalHit& hit) { return hit.getEnergy() > 0; });

  if (nhits < max_num_hits_) {
    // make inputs
    make_inputs(hexReadout, ecalRecHits);
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
    setStorageHint(hint_shouldKeep);
  } else {
    setStorageHint(hint_shouldDrop);
  }

  event.add(collectionName_, result);
}

void DNNEcalVetoProcessor::make_inputs(
    const EcalHexReadout& geom, const std::vector<EcalHit>& ecalRecHits) {
  // clear data
  for (auto& v : data_) {
    std::fill(v.begin(), v.end(), 0);
  }

  unsigned idx = 0;
  for (const auto& hit : ecalRecHits) {
    if (hit.getEnergy() <= 0) continue;
    double x = 0, y = 0, z = 0;
    geom.getCellAbsolutePosition(hit.getID(), x, y, z);

    data_[0].at(coordinate_x_offset_ + idx) = x;
    data_[0].at(coordinate_y_offset_ + idx) = y;
    data_[0].at(coordinate_z_offset_ + idx) = z;

    EcalID id(hit.getID());
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

}  // namespace ldmx

DECLARE_PRODUCER_NS(ldmx, DNNEcalVetoProcessor);
