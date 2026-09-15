#include "Tracking/Reco/TrigScintMeasurementProducer.h"

#include <algorithm>
#include <fstream>

#include <nlohmann/json.hpp>

#include "DetDescr/TrigScintGeometry.h"
#include "TSystem.h"
#include "TrigScint/Event/TrigScintQIEDigis.h"
#include "Tracking/Event/Measurement.h"

namespace tracking::reco {

void TrigScintMeasurementProducer::configure(
    framework::config::Parameters& parameters) {
  // Load the TrigScint_Event ROOT dictionary so the digi collection reads as the
  // compiled type. The accessors are inline (linker drops libTrigScint_Event)
  // and there is no rootmap for autoload, so ROOT would otherwise fall back to
  // an emulated streamer that crashes when cast to the compiled class.
  gSystem->Load("libTrigScint_Event");

  input_collections_ =
      parameters.getParameter<std::vector<std::string>>("input_collections");
  input_pass_ = parameters.getParameter<std::string>("input_pass", "");
  out_collection_ = parameters.getParameter<std::string>("out_collection",
                                                         "TrigScintMeasurements");
  amp_threshold_ = parameters.getParameter<double>("amp_threshold", amp_threshold_);
  sigma_y_ = parameters.getParameter<double>("sigma_y", sigma_y_);

  // Optional TS DAQ map: fixes the decode-collection -> geometry-module
  // (cabling) correspondence, analogous to the tracker's daq_map_file. Entries
  // with geo_module < 0 (e.g. the LYSO pad, not yet in the geometry) are
  // skipped.
  daq_map_file_ = parameters.getParameter<std::string>("daq_map_file", "");
  daq_modules_.clear();
  if (!daq_map_file_.empty()) {
    std::ifstream in(daq_map_file_);
    if (!in) {
      EXCEPTION_RAISE("TrigScintDaqMap",
                      "Could not open TS DAQ map '" + daq_map_file_ + "'.");
    }
    nlohmann::json doc;
    in >> doc;
    for (const auto& m : doc.at("modules")) {
      const int geo_module = m.at("geo_module").get<int>();
      if (geo_module < 0) continue;  // not mapped to geometry (e.g. LYSO)
      daq_modules_.emplace_back(m.at("collection").get<std::string>(),
                                geo_module);
    }
    ldmx_log(info) << "TS DAQ map '" << daq_map_file_ << "' -> "
                   << daq_modules_.size() << " mapped modules.";
  }
}

void TrigScintMeasurementProducer::produce(framework::Event& event) {
  // Bar positions come from the geometry conditions object (not hard-coded).
  const auto& geom = getCondition<ldmx::TrigScintGeometry>(
      ldmx::TrigScintGeometry::CONDITIONS_OBJECT_NAME);
  const int n_bars = geom.getNumBars();

  std::vector<ldmx::Measurement> measurements;

  // (decoded collection, geometry module) pairs to process: from the DAQ map
  // if provided, else the input_collections order (index = geometry module).
  std::vector<std::pair<std::string, int>> to_process = daq_modules_;
  if (to_process.empty()) {
    for (std::size_t i = 0; i < input_collections_.size(); ++i)
      to_process.emplace_back(input_collections_[i], static_cast<int>(i));
  }

  for (const auto& [collection, geo_module] : to_process) {
    const auto& digis = event.getCollection<trigscint::TrigScintQIEDigis>(
        collection, input_pass_);

    for (const auto& digi : digis) {
      const int chan = digi.getChanID();
      if (chan < 0 || chan >= n_bars) continue;

      const std::vector<int>& adc = digi.getADC();
      if (adc.empty()) continue;

      const auto max_it = std::max_element(adc.begin(), adc.end());
      const int amp = *max_it - *std::min_element(adc.begin(), adc.end());
      if (amp <= amp_threshold_) continue;  // not a hit

      // timing observable: which time sample the pulse peaks in [samples]
      const int peak_sample = static_cast<int>(std::distance(adc.begin(), max_it));

      // global bar-center position from the TrigScintGeometry conditions object
      const auto pos = geom.getBarPosition(geo_module, chan);

      ldmx::Measurement meas;
      meas.setGlobalPosition(static_cast<float>(pos.X()),
                             static_cast<float>(pos.Y()),
                             static_cast<float>(pos.Z()));
      meas.setLocalPosition(static_cast<float>(pos.Y()), 0.f);
      meas.setLocalCovariance(static_cast<float>(sigma_y_ * sigma_y_), 0.f);
      // encode geometry module + bar so downstream can trace the measurement
      meas.setLayerID(geo_module * 100 + chan);
      meas.setTime(static_cast<float>(peak_sample));  // peak time sample
      meas.setClusterAmplitude(static_cast<float>(amp));
      meas.setNStrips(1);
      measurements.push_back(meas);
    }
  }

  event.add(out_collection_, measurements);
}

}  // namespace tracking::reco

DECLARE_PRODUCER(tracking::reco::TrigScintMeasurementProducer)
