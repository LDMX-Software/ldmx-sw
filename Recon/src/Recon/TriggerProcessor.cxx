
#include "Recon/TriggerProcessor.h"

namespace recon {

void TriggerProcessor::configure(framework::config::Parameters& parameters) {
  layerESumCut_ = parameters.getParameter<double>("threshold");
  mode_ = parameters.getParameter<int>("mode");
  startLayer_ = parameters.getParameter<int>("start_layer");
  endLayer_ = parameters.getParameter<int>("end_layer");
  inputColl_ = parameters.getParameter<std::string>("input_collection");
  outputColl_ = parameters.getParameter<std::string>("trigger_collection");

  if (mode_ == 0) {
    algoName_ = "LayerSumTrig";
  } else if (mode_ == 1) {
    algoName_ = "CenterTower";
  }
}

void TriggerProcessor::produce(framework::Event& event) {
  /** Grab the Ecal hit collection for the given event */
  const std::vector<ldmx::EcalHit> ecalRecHits =
      event.getCollection<ldmx::EcalHit>(inputColl_);

  std::vector<double> layerDigiE(100, 0.0);  // big empty vector..

  /** Loop over all ecal hits in the given event */
  for (const ldmx::EcalHit& hit : ecalRecHits) {
    ldmx::EcalID id(hit.getID());
    if (id.layer() < layerDigiE.size()) {  // just to be safe...
      if (mode_ == 0) {  // Sum over all cells in a given layer
        layerDigiE[id.layer()] += hit.getEnergy();
      } else if (mode_ == 1) {  // Sum over cells in central tower only
                                // std::pair<float, float> xyPos =
        // hit->getCellCentroidXYPair(hit->getID()); float cellRadius =
        // sqrt(pow(xyPos.first, 2) + pow(xyPos.second, 2)); if (cellRadius <
        // MAGICNUMBERHERE) {
        //    layerDigiE[hit->getLayer()] += hit->getEnergy();
        //}
      }
    }
  }

  float layerSum = 0;
  bool pass = false;

  for (int iL = startLayer_; iL < endLayer_; ++iL) {
    layerSum += layerDigiE[iL];
  }

  pass = (layerSum <= layerESumCut_);

  ldmx::TriggerResult result;
  result.set(algoName_, pass, 3);
  result.setAlgoVar(0, layerSum);
  result.setAlgoVar(1, layerESumCut_);
  result.setAlgoVar(2, endLayer_ - startLayer_);

  event.add(outputColl_, result);

  // mark the event
  if (pass)
    setStorageHint(framework::hint_shouldKeep);
  else
    setStorageHint(framework::hint_shouldDrop);
}
}  // namespace recon

DECLARE_PRODUCER_NS(recon, TriggerProcessor)
