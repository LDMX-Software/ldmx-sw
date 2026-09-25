#include "Trigger/EcalNNTrigger.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>

#include "Recon/Event/TriggerResult.h"

namespace trigger {

namespace {

/// log-softmax of the logits
std::vector<double> logSoftmax(const std::vector<float>& logits) {
  const double max_val = *std::max_element(logits.begin(), logits.end());
  double sum_exp = 0.;
  for (const float logit : logits) sum_exp += std::exp(logit - max_val);
  const double log_sum_exp = max_val + std::log(sum_exp);

  std::vector<double> result;
  result.reserve(logits.size());
  for (const float logit : logits) result.push_back(logit - log_sum_exp);
  return result;
}

}  // namespace

EcalNNTrigger::EcalNNTrigger(const std::string& name,
                             framework::Process& process)
    : framework::Producer(name, process),
      data_{std::vector<float>(MAX_CLUSTERS * N_FEATURES, 0.f)} {}

void EcalNNTrigger::configure(framework::config::Parameters& ps) {
  max_pbkg_ = ps.get<double>("max_pbkg");
  cluster_coll_name_ = ps.get<std::string>("cluster_coll_name");
  ecal_sum_coll_name_ = ps.get<std::string>("ecal_sum_coll_name");
  hcal_sum_coll_name_ = ps.get<std::string>("hcal_sum_coll_name");
  input_pass_ = ps.get<std::string>("input_pass");
  trigger_coll_name_ = ps.get<std::string>("trigger_coll_name");

  const auto model_path{ps.get<std::string>("model_path")};
  rt_ = std::make_unique<ldmx::ort::ONNXRuntime>(model_path);

  // the batch dimension is reported as -1 by the wrapper
  const std::vector<int64_t> expected_shape{-1,
                                            static_cast<int64_t>(N_CLASSES)};
  const auto& outputs = rt_->getOutputNames();
  if (std::find(outputs.begin(), outputs.end(), "logits") == outputs.end() or
      rt_->getOutputShape("logits") != expected_shape) {
    EXCEPTION_RAISE("BadModel", "Model '" + model_path +
                                    "' does not have a 'logits' output of "
                                    "shape (batch, " +
                                    std::to_string(N_CLASSES) + ").");
  }
}

void EcalNNTrigger::produce(framework::Event& event) {
  // throws an exception if an input is missing or found in more than
  // one pass; an empty input is valid and the event is still scored
  const auto& clusters{
      event.getCollection<TrigCaloCluster>(cluster_coll_name_, input_pass_)};
  const auto& ecal_sums{
      event.getCollection<TrigEnergySum>(ecal_sum_coll_name_, input_pass_)};
  const auto& hcal_sums{
      event.getCollection<TrigEnergySum>(hcal_sum_coll_name_, input_pass_)};

  fillInputs(clusters, ecal_sums, hcal_sums, data_[0]);
  const auto logits{rt_->run({"x"}, data_, {"logits"})[0]};

  // compare P(bkg) directly: 1 - P(bkg) saturates to 1 in float
  const double p_bkg = std::exp(logSoftmax(logits)[0]);
  const bool pass = p_bkg <= max_pbkg_;

  ldmx_log(debug) << "n_clusters = " << clusters.size()
                  << ", P(bkg) = " << p_bkg << ", pass = " << pass;

  ldmx::TriggerResult result;
  result.set(trigger_coll_name_, pass, 2 + N_CLASSES);
  result.setAlgoVar(0, p_bkg);
  result.setAlgoVar(1, max_pbkg_);
  for (std::size_t i = 0; i < N_CLASSES; ++i) {
    result.setAlgoVar(2 + i, logits.at(i));
  }
  event.add(trigger_coll_name_, result);

  if (pass) {
    setStorageHint(framework::HINT_SHOULD_KEEP);
  } else {
    setStorageHint(framework::HINT_SHOULD_DROP);
  }
}

void EcalNNTrigger::fillInputs(const TrigCaloClusterCollection& clusters,
                               const TrigEnergySumCollection& ecal_sums,
                               const TrigEnergySumCollection& hcal_sums,
                               std::vector<float>& inputs) {
  inputs.assign(MAX_CLUSTERS * N_FEATURES, 0.f);

  // sort cluster indices by descending energy; stable so that ties keep
  // collection order, as in the training
  std::vector<std::size_t> order(clusters.size());
  std::iota(order.begin(), order.end(), 0);
  std::stable_sort(order.begin(), order.end(),
                   [&clusters](std::size_t a, std::size_t b) {
                     return clusters[a].e() > clusters[b].e();
                   });

  double ecal_sum_e = 0.;
  for (const auto& sum : ecal_sums) ecal_sum_e += sum.energy();
  // the HCal sums carry hardware (ADC) energy; energy() is not filled
  int hcal_sum_adc = 0;
  for (const auto& sum : hcal_sums) hcal_sum_adc += sum.hwEnergy();

  for (std::size_t slot = 0; slot < MAX_CLUSTERS; ++slot) {
    float* feats = &inputs[slot * N_FEATURES];
    if (slot < order.size()) {
      const auto& cluster = clusters[order[slot]];
      feats[0] = cluster.x();
      feats[1] = cluster.y();
      feats[2] = cluster.z();
      feats[3] = cluster.e();
      feats[4] = cluster.depth();
      feats[5] = cluster.nTP();
    }
    // event-level features are filled in every slot, even empty ones
    feats[6] = clusters.size();
    feats[7] = ecal_sum_e;
    feats[8] = hcal_sum_adc;
  }
}

}  // namespace trigger

DECLARE_PRODUCER(trigger::EcalNNTrigger);
