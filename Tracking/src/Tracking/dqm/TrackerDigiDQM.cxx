#include "Tracking/dqm/TrackerDigiDQM.h"

#include "SimCore/Event/SimParticle.h"
#include "Tracking/Event/Measurement.h"

namespace tracking::dqm {

void TrackerDigiDQM::configure(framework::config::Parameters& parameters) {
  output_measurements_passname_ = parameters.getParameter<std::string>("output_measurements_passname"); 
  measurements_passname_ = parameters.getParameter<std::string>("measurements_passname");
}

void TrackerDigiDQM::analyze(const framework::Event& event) {
  if (!event.exists("OutputMeasurements", output_measurements_passname_)) return;
  auto measurements{
      event.getCollection<ldmx::Measurement>("OutputMeasurements", measurements_passname_)};

  for (auto& measurement : measurements) {
    auto global_position{measurement.getGlobalPosition()};
    auto local_position{measurement.getLocalPosition()};
    auto layer_id{measurement.getLayerID()};
    auto time{measurement.getTime()};

    histograms_.fill("global_yz_l" + std::to_string(layer_id),
                     global_position[1], global_position[2]);
    histograms_.fill("global_xy", global_position[0], global_position[1]);

    histograms_.fill("local_uv_l" + std::to_string(layer_id), local_position[0],
                     local_position[1]);

    histograms_.fill("time_l" + std::to_string(layer_id), time);
  }
}
}  // namespace tracking::dqm

DECLARE_ANALYZER(tracking::dqm::TrackerDigiDQM)
