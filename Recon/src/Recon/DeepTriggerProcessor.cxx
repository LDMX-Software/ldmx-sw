/**
 * @file DeepTriggerProcessor.cxx
 * @brief Class that provides a deep-layer trigger decision for recon using a
 * TriggerResult object
 * @author Hrideyesh Tyagi, UC Santa Barbara
 *
 * Based on TriggerProcessor by Josh Hiltbrand (University of Minnesota) and
 * Lene Kristian Bryngemark (Stanford University). Identical except that an
 * event passes if the layer energy sum is at least the threshold.
 */

#include <limits>

#include "DetDescr/EcalID.h"
#include "Ecal/Event/EcalHit.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Recon/Event/TriggerResult.h"

namespace recon {

/**
 * @class DeepTriggerProcessor
 * @brief Provides a deep-layer trigger decision for recon using a
 * TriggerResult object.
 *
 * @note
 * DeepTriggerProcessor takes in a set of parameters to be used in defining
 * the trigger algorithm. An event is passed to the processor and the relevant
 * algorithms are then run on the event (ECAL layer sum). The event passes if
 * the layer sum is at least the threshold, e.g. to select events depositing
 * energy in the back layers of the ECAL. A trigger decision is executed and
 * the decision along with the algorithm name and relevant variables are
 * stored in a TriggerResult object which is added to the collection.
 */
class DeepTriggerProcessor : public framework::Producer {
 public:
  /**
   * Class constructor.
   */
  DeepTriggerProcessor(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  /**
   * Class destructor.
   */
  virtual ~DeepTriggerProcessor() {}

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters& parameters) override;

  /**
   * Run the trigger algorithm and create a TriggerResult
   * object to contain info about the trigger decision
   * such as pass/fail, number of saved variables,
   * etc.
   * param event The event to run trigger algorithm on.
   */
  void produce(framework::Event& event) override;

 private:
  /// The energy sum to make cut on.
  std::vector<double> layer_e_sum_cuts_;

  /// The Beam energy [MeV]
  double beam_energy_;

  /**
   * The trigger mode to run in. Mode zero sums over
   * all cells in layer_, while in mode 1 only cells in
   * center module are summed over. (TODO)
   */
  int mode_{0};

  /** The first layer of layer sum. */
  int start_layer_{0};

  /**
   * The endpoint layer of layer sum.
   *
   * **not inclusive** - i.e. this is the first layer that
   * is not included in the layer sum.
   */
  int end_layer_{0};

  /** The name of the trigger algorithm used. */
  TString algo_name_;

  /** The name of the input collection (the Ecal hits_). */
  std::string input_coll_;

  /** The pass name of the input (the Ecal hits_). */
  std::string input_pass_;

  /** The name of the output collection (the trigger decision). */
  std::string output_coll_;
};

void DeepTriggerProcessor::configure(
    framework::config::Parameters& parameters) {
  layer_e_sum_cuts_ = parameters.get<std::vector<double>>("thresholds");
  beam_energy_ = parameters.get<double>("beam_energy");
  mode_ = parameters.get<int>("mode");
  start_layer_ = parameters.get<int>("start_layer");
  end_layer_ = parameters.get<int>("end_layer");
  input_coll_ = parameters.get<std::string>("input_collection");
  input_pass_ = parameters.get<std::string>("input_pass");
  output_coll_ = parameters.get<std::string>("trigger_collection");

  if (mode_ == 0) {
    algo_name_ = "DeepLayerSumTrig";
  } else if (mode_ == 1) {
    algo_name_ = "DeepCenterTower";
  }
}

void DeepTriggerProcessor::produce(framework::Event& event) {
  /** Grab the Ecal hit collection for the given event */
  const std::vector<ldmx::EcalHit> ecal_rec_hits =
      event.getCollection<ldmx::EcalHit>(input_coll_, input_pass_);

  // number of electrons in this event
  const int n_electrons{event.getElectronCount()};

  /**
   * There are three scenarios:
   *  1. No Incoming Electrons - If the electron count is 0 or negative
   *     (undetermined), then we set the sum-energy cut to the largest
   *     possible value so the event always fails.
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
    layer_e_sum_cut =
        std::numeric_limits<double>::max();  // always fail if no electrons
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
        // not implemented, see TriggerProcessor
      }
    }
  }

  float layer_sum = 0;
  bool pass = false;

  for (int i_l = start_layer_; i_l < end_layer_; ++i_l) {
    layer_sum += layer_digi_e[i_l];
  }

  pass = (layer_sum >= layer_e_sum_cut);
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

DECLARE_PRODUCER(recon::DeepTriggerProcessor)
