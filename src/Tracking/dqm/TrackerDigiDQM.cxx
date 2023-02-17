#include "Tracking/dqm/TrackerDigiDQM.h" 

#include "SimCore/Event/SimParticle.h"
#include "Tracking/Event/Measurement.h"

namespace tracking::dqm { 

void TrackerDigiDQM::analyze(const framework::Event& event) { 

  if (!event.exists("OutputMeasurements")) return; 
  auto measurements{event.getCollection<ldmx::Measurement>("OutputMeasurements")}; 

  for (auto& measurement : measurements) { 
    auto global_position{measurement.getGlobalPosition()};
    auto local_position{measurement.getLocalPosition()}; 
    auto layer_id{measurement.getLyID()};
    auto time{measurement.getTime()}; 
    
    histograms_.fill("global_yz_l"+std::to_string(layer_id), global_position[1], global_position[2]);
    histograms_.fill("global_xy", global_position[0], global_position[1]); 
    
    histograms_.fill("local_uv_l"+std::to_string(layer_id), local_position[0], local_position[1]);

    histograms_.fill("time_l"+std::to_string(layer_id), time); 

  }  
  
}
}

DECLARE_ANALYZER_NS(tracking::dqm, TrackerDigiDQM)
