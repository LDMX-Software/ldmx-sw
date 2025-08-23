
#include "Recon/TriggerProcessor.h"

#include "DetDescr/EcalID.h"

namespace recon {

void TriggerProcessor::configure(framework::config::Parameters& parameters) {
  layer_e_sum_cuts_ = parameters.get<std::vector<double>>("thresholds");
  beam_energy_ = parameters.get<double>("beamEnergy");
  mode_ = parameters.get<int>("mode");
  start_layer_ = parameters.get<int>("start_layer");
  end_layer_ = parameters.get<int>("end_layer");
  input_coll_ = parameters.get<std::string>("input_collection");
  input_pass_ = parameters.get<std::string>("input_pass");
  output_coll_ = parameters.get<std::string>("trigger_collection");

  if (mode_ == 0) {
    algo_name_ = "LayerSumTrig";
  } else if (mode_ == 1) {
    algo_name_ = "CenterTower";
  }
}

void TriggerProcessor::produce(framework::Event& event) {
  /** Grab the Ecal hit collection for the given event */
  const std::vector<ldmx::EcalHit> ecal_rec_hits =
      event.getCollection<ldmx::EcalHit>(input_coll_, input_pass_);

  // number of electrons in this event
  const int n_electrons{event.getElectronCount()};

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
  double layer_e_sum_cut{0.};
  if (n_electrons <= 0)
    layer_e_sum_cut = 0.;  // always fail if no electrons
  else if (n_electrons <= layer_e_sum_cuts_.size())
    layer_e_sum_cut = layer_e_sum_cuts_.at(n_electrons - 1);
  else
    layer_e_sum_cut =
        layer_e_sum_cuts_.at(0) + (n_electrons - 1) * beam_energy_;

  ldmx_log(info) << "Got trigger energy cut " << layer_e_sum_cut << " for "
                 << n_electrons << " electrons counted in the event.";

  std::vector<double> layer_digi_e(100, 0.0);  // big empty vector..

  /** Loop over all ecal hits_ in the given event */
  for (const ldmx::EcalHit& hit : ecal_rec_hits) {
    ldmx::EcalID id(hit.getID());
    if (id.layer() < layer_digi_e.size()) {  // just to be safe...
      if (mode_ == 0) {  // Sum over all cells in a given layer_
        layer_digi_e[id.layer()] += hit.getEnergy();
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

  float layer_sum = 0;
  bool pass = false;

  for (int i_l = start_layer_; i_l < end_layer_; ++i_l) {
    layer_sum += layer_digi_e[i_l];
  }

  pass = (layer_sum <= layer_e_sum_cut);
  ldmx_log(info) << "Got trigger energy sum " << layer_sum
                 << "; and decision is pass = " << pass;

  ldmx::TriggerResult result;
  result.set(algo_name_, pass, 4);
  result.setAlgoVar(0, layer_sum);
  result.setAlgoVar(1, layer_e_sum_cut);
  result.setAlgoVar(2, end_layer_ - start_layer_);
  result.setAlgoVar(3, n_electrons);

  event.add(output_coll_, result);

  // mark the event
  if (pass)
    setStorageHint(framework::HINT_SHOULD_KEEP);
  else
    setStorageHint(framework::HINT_SHOULD_DROP);
}
}  // namespace recon

DECLARE_PRODUCER(recon::TriggerProcessor)
