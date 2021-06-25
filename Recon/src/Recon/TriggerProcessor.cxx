
#include "Recon/TriggerProcessor.h"

namespace recon {

void TriggerProcessor::configure(framework::config::Parameters& parameters) {
  layerESumCuts_ = parameters.getParameter<std::vector<double>>("thresholds");
  beamEnergy_ = parameters.getParameter<double>("beamEnergy");
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

  // number of electrons in this event
  const int nElectrons{event.getElectronCount()};

  /**
   * There are three scenarios:
   *  1. No Incoming Electrons - If the electron count is 0 or negative
   *     (undetermined), then we set the sum-energy cut to zero 
   *     so the event always fails.
   *  2. Num Electrons Listed in Thresholds - Pull cut from list
   *  3. More electrons than listed - Set threshold as
   *      'threshold_for_1e + nExtraElectrons*beamEnergy'
   *     Note that the "overflow" formula here is too naive.
   *     It should be a
   *      fct( nElectrons, 1e_thr, beamE),
   *     taking how sigma evolves with multiplicity into account.
   *     a simple scaling might suffice there too assuming 
   *     energy cuts are listed as [ Ecut_1e, Ecut_2e, ... ]
   */
  double layerESumCut{0.};
  if (nElectrons <= 0)
    layerESumCut = 0.; // always fail if no electrons
  else if (nElectrons <= layerESumCuts_.size())
    layerESumCut = layerESumCuts_.at(nElectrons - 1);
  else
    layerESumCut = layerESumCuts_.at(0) + (nElectrons - 1) * beamEnergy_;

  ldmx_log(debug) << "Got trigger energy cut " << layerESumCut << " for "
                  << nElectrons << " electrons counted in the event.";

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

  pass = (layerSum <= layerESumCut);
  ldmx_log(debug) << "Got trigger energy sum " << layerSum
                  << "; and decision is pass = " << pass;

  ldmx::TriggerResult result;
  result.set(algoName_, pass, 4);
  result.setAlgoVar(0, layerSum);
  result.setAlgoVar(1, layerESumCut);
  result.setAlgoVar(2, endLayer_ - startLayer_);
  result.setAlgoVar(3, nElectrons);

  event.add(outputColl_, result);

  // mark the event
  if (pass)
    setStorageHint(framework::hint_shouldKeep);
  else
    setStorageHint(framework::hint_shouldDrop);
}
}  // namespace recon

DECLARE_PRODUCER_NS(recon, TriggerProcessor)
