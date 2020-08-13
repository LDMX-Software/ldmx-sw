#include "Ecal/DNNEcalVetoProcessor.h"

// LDMX
#include "Event/EcalHit.h"
#include "Event/EventConstants.h"

#include <algorithm>

namespace ldmx {

  const std::vector<std::string> DNNEcalVetoProcessor::input_names_ {"coordinates", "features"};
  const std::vector<unsigned int> DNNEcalVetoProcessor::input_sizes_ {
    n_coordinate_dim_*max_num_hits_, n_feature_dim_*max_num_hits_};

  DNNEcalVetoProcessor::DNNEcalVetoProcessor(const std::string& name, Process& process) :
      Producer(name, process) {
    for (const auto &s : input_sizes_){
      data_.emplace_back(s, 0);
    }
  }

  void DNNEcalVetoProcessor::configure(Parameters &parameters) {
    // These are the v12 parameters
    //  all distances in mm
    double moduleRadius = 85.0; //same as default
    int    numCellsWide = 23; //same as default
    double moduleGap = 1.5;
    double ecalFrontZ = 220;
    std::vector<double> ecalSensLayersZ = {
         7.850,
        13.300,
        26.400,
        33.500,
        47.950,
        56.550,
        72.250,
        81.350,
        97.050,
        106.150,
        121.850,
        130.950,
        146.650,
        155.750,
        171.450,
        180.550,
        196.250,
        205.350,
        221.050,
        230.150,
        245.850,
        254.950,
        270.650,
        279.750,
        298.950,
        311.550,
        330.750,
        343.350,
        362.550,
        375.150,
        394.350,
        406.950,
        426.150,
        438.750
    };

    hexReadout_ = std::make_unique<EcalHexReadout>(
            moduleRadius,
            moduleGap,
            numCellsWide,
            ecalSensLayersZ,
            ecalFrontZ
            );

    disc_cut_ = parameters.getParameter<double>("disc_cut");
#ifdef LDMX_USE_ONNXRUNTIME
    rt_ = std::make_unique<Ort::ONNXRuntime>(parameters.getParameter<std::string>("model_path"));
#else
    EXCEPTION_RAISE("DNNEcalVetoProcessor",
                    "Cannot run DNN because ONNXRuntime is not installed.");
#endif

    // debug mode
    debug_ = parameters.getParameter<bool>("debug");
    if (debug_) {
      std::cout << "=== Module position map ===" << std::endl;
      for (const auto &p : hexReadout_->getCellModulePositionMap()) {
        std::cout << p.first << " " << p.second.first << " " << p.second.second << std::endl;
      }
      std::cout << "=== Layer Zs ===" << std::endl;
      for (unsigned layer=0; layer<34; ++layer) {
        std::cout << hexReadout_->getZPosition(layer) << std::endl;
      }
      std::cout << std::endl << "========================" << std::endl;
    }

    // Set the collection name as defined in the configuration
    collectionName_ = parameters.getParameter<std::string>("collection_name");
  }

  void DNNEcalVetoProcessor::produce(Event &event) {
    EcalVetoResult result;

    // Get the collection of digitized Ecal hits from the event.
    const auto ecalRecHits = event.getCollection<EcalHit>("EcalRecHits");
    auto nhits = std::count_if(ecalRecHits.begin(), ecalRecHits.end(), [](const EcalHit& hit){ return hit.getEnergy()>0; });

    if (nhits < max_num_hits_) {
      // make inputs
      make_inputs(ecalRecHits);
      // run the DNN
#ifdef LDMX_USE_ONNXRUNTIME
      auto outputs = rt_->run(input_names_, data_)[0];
      result.setDiscValue(outputs.at(1));
#endif
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

  void DNNEcalVetoProcessor::make_inputs(const std::vector<EcalHit>& ecalRecHits) {
    // clear data
    for (auto &v : data_){
      std::fill(v.begin(), v.end(), 0);
    }

    unsigned idx = 0;
    for (const auto& hit : ecalRecHits) {
      if (hit.getEnergy() <= 0) continue;
      double x = 0, y = 0, z = 0;
      hexReadout_->getCellAbsolutePosition(hit.getID(), x, y, z);

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
      for (unsigned iname=0; iname<input_names_.size(); ++iname) {
        std::cout << "=== " << input_names_[iname] << " ===" << std::endl;
        for (unsigned i=0; i<input_sizes_[iname]; ++i){
          std::cout << data_[iname].at(i) << ", ";
          if ((i + 1) % max_num_hits_ == 0) {
            std::cout << std::endl;
          }
        }
      }
    } // debug

  }


}


DECLARE_PRODUCER_NS(ldmx, DNNEcalVetoProcessor);

