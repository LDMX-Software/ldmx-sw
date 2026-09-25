/**
 * @file TrigClusterNNProducer.h
 * @brief ECal NN classifier score from an MLP over trigger clusters and
 * calorimeter sums
 * @author Charles Bell
 */

#ifndef TRIGGER_TRIGCLUSTERNNPRODUCER_H
#define TRIGGER_TRIGCLUSTERNNPRODUCER_H

#include <chrono>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

// LDMX
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Tools/ONNXRuntime.h"
#include "Trigger/Event/TrigCaloCluster.h"
#include "Trigger/Event/TrigEnergySum.h"

namespace trigger {

/**
 * @class TrigClusterNNProducer
 * @brief Scores each event with the ECal trigger MLP and stores the output
 *
 * The model input is the top-3 (energy-sorted) ECal trigger clusters times 9
 * features: x, y, z, e, depth, n_tp of the cluster, then the event-level
 * n_clusters (before truncation), the sum of the ECal trigger sums (MeV) and
 * the sum of the HCal back-layer trigger sums (ADC), repeated in every slot.
 * The scaler is part of the ONNX graph, so the inputs are raw values.
 *
 * Timing: this reproduces the in-time bunch-crossing decision only. All inputs
 * are built from the sample of interest of the ECal/HCal digis, as in the
 * training samples. Trigger latency and out-of-time pileup are not modelled.
 *
 * Required upstream sequence with default parameters to match training:
 * EcalTrigPrimDigiProducer, HcalTrigPrimDigiProducer, EcalTPSelector,
 * TrigHcalEnergySum and TrigEcalClusterProducer. A missing input collection
 * throws an exception but an empty one is still scored.
 *
 * Output: a TrigClusterNNScore with the raw logits and P(bkg).
 */
class TrigClusterNNProducer : public framework::Producer {
 public:
  TrigClusterNNProducer(const std::string& name, framework::Process& process);
  void configure(framework::config::Parameters& ps) override;
  void produce(framework::Event& event) override;
  void onProcessEnd() override;

  /**
   * Fill flattened (MAX_CLUSTERS, N_FEATURES) model input,
   * (index = slot * N_FEATURES + feature). Unused cluster slots are zero.
   *
   * @param[in] clusters ECal trigger clusters, in collection order
   * @param[in] ecal_sums ECal trigger energy sums (energy() is used)
   * @param[in] hcal_sums HCal back-layer trigger sums (hwEnergy() is used)
   * @param[out] inputs resized to MAX_CLUSTERS * N_FEATURES and filled
   */
  static void fillInputs(const TrigCaloClusterCollection& clusters,
                         const TrigEnergySumCollection& ecal_sums,
                         const TrigEnergySumCollection& hcal_sums,
                         std::vector<float>& inputs);

  static constexpr std::size_t MAX_CLUSTERS = 3;
  static constexpr std::size_t N_FEATURES = 9;
  static constexpr std::size_t N_CLASSES = 9;

 private:
  std::string cluster_coll_name_;
  std::string ecal_sum_coll_name_;
  std::string hcal_sum_coll_name_;
  /// pass name shared by all three input collections
  std::string input_pass_;
  std::string score_coll_name_;

  /// model input buffer, reused every event
  ldmx::ort::FloatArrays data_;
  std::unique_ptr<ldmx::ort::ONNXRuntime> rt_;

  /// profiling: events seen and accumulated wall time (ms)
  int nevents_{0};
  double processing_time_{0.};
  double inference_time_{0.};
};

}  // namespace trigger

#endif  // TRIGGER_TRIGCLUSTERNNPRODUCER_H
