#include "BeamInstrumentation/FiberTrackerDecoder.h"

namespace beaminstrumentation {

  void FiberTrackerDecoder::configure(framework::config::Parameters &ps){
    outputCollection_ = ps.getParameter<std::string>("output_collection");
    inputCollectionDownstreamHorizontal_ = ps.getParameter<std::string>("input_collection_downstream_horizontal");
    inputCollectionDownstreamVertical_ = ps.getParameter<std::string>("input_collection_downstream_vertical");
    inputCollectionUpstreamHorizontal_ = ps.getParameter<std::string>("input_collection_upstream_horizontal");
    inputCollectionUpstreamVertical_ = ps.getParameter<std::string>("input_collection_upstream_vertical");
    inputPassName_ = ps.getParameter<std::string>("input_pass_name");
  }

  void FiberTrackerDecoder::produce(framework::Event &event){

    const auto eventStreamDownstreamHorizontal{event.getCollection<uint8_t>( inputCollectionDownstreamHorizontal_, inputPassName_)}; 
    const auto eventStreamDownstreamVertical{event.getCollection<uint8_t>( inputCollectionDownstreamVertical_, inputPassName_)}; 
    const auto eventStreamUpstreamHorizontal{event.getCollection<uint8_t>( inputCollectionUpstreamHorizontal_, inputPassName_)}; 
    const auto eventStreamUpstreamVertical{event.getCollection<uint8_t>( inputCollectionUpstreamVertical_, inputPassName_)}; 

    std::vector<uint> hitsDownstreamHorizontal;
    if(eventStreamDownstreamHorizontal.size() == 40){
      for(int i = 0; i < 24; i++){
        for(int k = 0; k < 8; k++){
          if((eventStreamDownstreamHorizontal.at(16+i) >> k) & 0x1 == 1){
            hitsDownstreamHorizontal.push_back(i*8+k);
          }
        }
      }
    }

    std::vector<uint> hitsDownstreamVertical;
    if(eventStreamDownstreamVertical.size() == 40){
      for(int i = 0; i < 24; i++){
        for(int k = 0; k < 8; k++){
          if((eventStreamDownstreamVertical.at(16+i) >> k) & 0x1 == 1){
            hitsDownstreamVertical.push_back(i*8+k);
          }
        }
      }
    }

    std::vector<uint> hitsUpstreamHorizontal;
    if(eventStreamUpstreamHorizontal.size() == 40){
      for(int i = 0; i < 24; i++){
        for(int k = 0; k < 8; k++){
          if((eventStreamUpstreamHorizontal.at(16+i) >> k) & 0x1 == 1){
            hitsUpstreamHorizontal.push_back(i*8+k);
          }
        }
      }
    }

    std::vector<uint> hitsUpstreamVertical;
    if(eventStreamUpstreamVertical.size() == 40){
      for(int i = 0; i < 24; i++){
        for(int k = 0; k < 8; k++){
          if((eventStreamUpstreamVertical.at(16+i) >> k) & 0x1 == 1){
            hitsUpstreamVertical.push_back(i*8+k);
          }
        }
      }
    }

    FiberTracker out(hitsDownstreamHorizontal, hitsDownstreamVertical, hitsUpstreamHorizontal,  hitsUpstreamVertical);

    event.add(outputCollection_, out);
  }

  void FiberTrackerDecoder::onFileOpen() {
    ldmx_log(debug) << "FiberTrackerDecoder Opening file!";

    return;
  } 

  void FiberTrackerDecoder::onFileClose() {
    ldmx_log(debug) << "FiberTrackerDecoder Closing file!";

    return;
  }

  void FiberTrackerDecoder::onProcessStart() {
    ldmx_log(debug) << "FiberTrackerDecoder Process start!";

    return;
  }

  void FiberTrackerDecoder::onProcessEnd(){
    ldmx_log(debug) << "FiberTrackerDecoder Process ends!";

    return;
  }

} // namespace beaminstrumentation

DECLARE_PRODUCER_NS(beaminstrumentation, FiberTrackerDecoder);
