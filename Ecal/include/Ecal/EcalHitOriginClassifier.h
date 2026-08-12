#ifndef ECAL_ECALHITORIGINCLASSIFIER_H_
#define ECAL_ECALHITORIGINCLASSIFIER_H_

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Ecal/Event/EcalHit.h"
#include "Ecal/Event/EcalHitClassification.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Tools/ONNXRuntime.h"
#include "TrigScint/Event/TrigScintTrack.h"

namespace ecal {

/** Run the ECal hit-origin ONNX classifier. */
class EcalHitOriginClassifier : public framework::Producer {
 public:
  EcalHitOriginClassifier(const std::string& name, framework::Process& process);
  ~EcalHitOriginClassifier() override = default;

  void configure(framework::config::Parameters& parameters) override;
  void produce(framework::Event& event) override;

 private:
  using FloatArrays = ldmx::ort::FloatArrays;
  using ShapeArrays = ldmx::ort::ShapeArrays;

  enum class FeatureLayout { EcalOnly, EcalTpad };

  struct DecodedOutput {
    std::vector<ldmx::EcalHitClassification> classifications;
    std::size_t num_classes{0};
  };

  void makeInputs(const std::vector<ldmx::EcalHit>& hits,
                  const std::vector<ldmx::TrigScintTrack>& tpad_tracks);
  void preprocessRow(std::vector<double>& row) const;

  DecodedOutput decodeOutputs(const std::vector<ldmx::EcalHit>& hits,
                              std::size_t num_tokens,
                              const FloatArrays& outputs) const;

  void addTruth(const framework::Event& event, std::size_t model_class_count,
                std::vector<ldmx::EcalHitClassification>& classifications) const;

  static std::pair<int, float> classify(const float* logits,
                                        std::size_t num_classes);

  std::string ecal_hit_collection_{"EcalRecHits"};
  std::string ecal_hit_pass_name_{};
  std::string trigger_pad_collection_{"TriggerPadTracks"};
  std::string trigger_pad_pass_name_{};
  std::string output_collection_{"EcalHitClassifications"};
  std::string model_path_{};
  std::string input_name_{"tokens"};
  std::string output_name_{"logits"};

  bool use_trigger_pad_context_{false};
  bool output_includes_context_tokens_{false};
  bool apply_log1p_ecal_energy_{false};
  bool apply_log1p_trigger_pad_pe_{false};

  std::vector<double> feature_means_{};
  std::vector<double> feature_stds_{};

  bool enable_truth_{false};
  std::string sim_hit_collection_{"EcalSimHitsOverlay"};
  std::string sim_hit_pass_name_{"overlay"};
  std::vector<int> expected_origin_ids_{1, 2, 3};
  double truth_energy_tie_relative_tolerance_{1.0e-4};

  FeatureLayout feature_layout_{FeatureLayout::EcalOnly};
  std::size_t feature_count_{4};
  std::size_t ecal_energy_column_{3};
  std::size_t tpad_pe_column_{0};

  FloatArrays input_data_{};
  ShapeArrays input_shapes_{};
  std::unique_ptr<ldmx::ort::ONNXRuntime> runtime_{};
};

}  // namespace ecal

#endif  // ECAL_ECALHITORIGINCLASSIFIER_H_
