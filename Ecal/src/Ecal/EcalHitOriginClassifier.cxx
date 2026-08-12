#include "Ecal/EcalHitOriginClassifier.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "Framework/Exception/Exception.h"
#include "SimCore/Event/SimCalorimeterHit.h"

namespace {

struct OriginContribution {
  double energy= 0.;
  unsigned int count=0;
};

struct HitTruth {
  int origin_id{-1};
  float fraction{0.0f};
};

HitTruth dominantOrigin(const std::map<int, OriginContribution>& contributions,
                        double relative_tolerance) {
  HitTruth truth;
  if (contributions.empty())
    return truth;

  double total_energy= 0.;
  double maximum_energy= 0.;
  for (const auto& [origin_id, contribution] : contributions) {
    (void)origin_id;
    total_energy += contribution.energy;
    maximum_energy = std::max(maximum_energy, contribution.energy);
  }

  unsigned int fewest_contributions{0};
  bool found{false};
  double winner_energy= 0.;
  for (const auto& [origin_id, contribution] : contributions) {
    double scale = std::max(std::abs(contribution.energy),
                                  std::abs(maximum_energy));
    if (std::abs(contribution.energy - maximum_energy) >
        relative_tolerance * scale) {
      continue;
    }

    if (!found || contribution.count < fewest_contributions) {
      truth.origin_id = origin_id;
      winner_energy = contribution.energy;
      fewest_contributions = contribution.count;
      found = true;
    }
  }

  if (found) truth.fraction = static_cast<float>(winner_energy / total_energy);
  return truth;
}

std::map<int, int> truthClasses(
    const std::map<int, std::pair<double, double>>& y_sums,
    const std::vector<int>& expected_origin_ids, std::size_t model_class_count) {
  std::map<int, int> result;
  if (model_class_count != expected_origin_ids.size()) return result;

  std::vector<std::pair<double, int>> ordered_origins;
  ordered_origins.reserve(expected_origin_ids.size());
  for (const int origin_id : expected_origin_ids) {
    const auto it = y_sums.find(origin_id);
    if (it == y_sums.end()) return {};
    ordered_origins.emplace_back(it->second.second / it->second.first,
                                 origin_id);
  }

  std::sort(ordered_origins.begin(), ordered_origins.end());
  for (std::size_t i = 0; i < ordered_origins.size(); i++) {
    result[ordered_origins[i].second] = static_cast<int>(i + 1);
  }
  return result;
}

}  // namespace

namespace ecal {

EcalHitOriginClassifier::EcalHitOriginClassifier(
    const std::string& name, framework::Process& process)
    : framework::Producer(name, process) {}

void EcalHitOriginClassifier::configure(
    framework::config::Parameters& parameters) {
  ecal_hit_collection_ = parameters.get<std::string>("ecal_hit_collection");
  ecal_hit_pass_name_ = parameters.get<std::string>("ecal_hit_pass_name");
  trigger_pad_collection_ =
      parameters.get<std::string>("trigger_pad_collection");
  trigger_pad_pass_name_ =
      parameters.get<std::string>("trigger_pad_pass_name");
  output_collection_ = parameters.get<std::string>("output_collection");
  model_path_ = parameters.get<std::string>("model_path");
  input_name_ = parameters.get<std::string>("input_name");
  output_name_ = parameters.get<std::string>("output_name");
  use_trigger_pad_context_ =
      parameters.get<bool>("use_trigger_pad_context");
  output_includes_context_tokens_ =
      parameters.get<bool>("output_includes_context_tokens");
  apply_log1p_ecal_energy_ =
      parameters.get<bool>("apply_log1p_ecal_energy");
  apply_log1p_trigger_pad_pe_ =
      parameters.get<bool>("apply_log1p_trigger_pad_pe");

  feature_means_ = parameters.get<std::vector<double>>("feature_means", std::vector<double>{});
  feature_stds_ = parameters.get<std::vector<double>>("feature_stds", std::vector<double>{});

  enable_truth_ = parameters.get<bool>("enable_truth");
  sim_hit_collection_ = parameters.get<std::string>("sim_hit_collection");
  sim_hit_pass_name_ = parameters.get<std::string>("sim_hit_pass_name");
  expected_origin_ids_ =
      parameters.get<std::vector<int>>("expected_origin_ids");
  truth_energy_tie_relative_tolerance_ =
      parameters.get<double>("truth_energy_tie_relative_tolerance");

  if (use_trigger_pad_context_) {
    feature_layout_ = FeatureLayout::EcalTpad;
    feature_count_ = 8;
    ecal_energy_column_ = 5;
    tpad_pe_column_ = 7;
  } else {
    feature_layout_ = FeatureLayout::EcalOnly;
    feature_count_ = 4;
    ecal_energy_column_ = 3;
  }

  if ((!feature_means_.empty() && feature_means_.size() != feature_count_) ||
      feature_stds_.size() != feature_means_.size()) {
    EXCEPTION_RAISE("InvalidFeatureNormalization",
                    "feature_means and feature_stds must match the model input size.");
  }

  runtime_ = std::make_unique<ldmx::ort::ONNXRuntime>(model_path_);
}

void EcalHitOriginClassifier::produce(framework::Event& event) {
  const auto& hits = event.getCollection<ldmx::EcalHit>(
      ecal_hit_collection_, ecal_hit_pass_name_);

  std::vector<ldmx::EcalHitClassification> classifications;
  if (!hits.empty()) {
    const std::vector<ldmx::TrigScintTrack> no_tpad_tracks;
    const std::vector<ldmx::TrigScintTrack>* tpad_tracks = &no_tpad_tracks;
    if (use_trigger_pad_context_) {
      tpad_tracks = &event.getCollection<ldmx::TrigScintTrack>(
          trigger_pad_collection_, trigger_pad_pass_name_);
    }

    makeInputs(hits, *tpad_tracks);
    const auto outputs = runtime_->run({input_name_}, input_data_, input_shapes_,
                                       {output_name_});
    const std::size_t num_tokens = hits.size() + tpad_tracks->size();
    auto decoded = decodeOutputs(hits, num_tokens, outputs);
    classifications = std::move(decoded.classifications);

    if (enable_truth_) addTruth(event, decoded.num_classes, classifications);
  }

  event.add(output_collection_, classifications);
}

void EcalHitOriginClassifier::makeInputs(
    const std::vector<ldmx::EcalHit>& hits,
    const std::vector<ldmx::TrigScintTrack>& tpad_tracks) {
  std::vector<float> features;
  features.reserve((hits.size() + tpad_tracks.size()) * feature_count_);

  const auto append_row = [this, &features](std::vector<double> row) {
    preprocessRow(row);
    for (const double value : row) features.push_back(static_cast<float>(value));
  };

  for (const auto& hit : hits) {
    if (feature_layout_ == FeatureLayout::EcalOnly) {
      append_row({hit.getXPos(), hit.getYPos(), hit.getZPos(), hit.getEnergy()});
    } else {
      append_row({1.0, 0.0, hit.getXPos(), hit.getYPos(), hit.getZPos(),
                  hit.getEnergy(), 0.0, 0.0});
    }
  }

  if (feature_layout_ == FeatureLayout::EcalTpad) {
    for (const auto& track : tpad_tracks) {
      append_row({0.0, 1.0, 0.0, 0.0, 0.0, 0.0, track.getCentroid(),
                  track.getPE()});
    }
  }

  input_data_.clear();
  input_data_.emplace_back(std::move(features));
  input_shapes_ = {{static_cast<int64_t>(hits.size() + tpad_tracks.size()),
                    static_cast<int64_t>(feature_count_)}};
}

void EcalHitOriginClassifier::preprocessRow(std::vector<double>& row) const {
  if (apply_log1p_ecal_energy_ && row[ecal_energy_column_] != 0.0) {
    row[ecal_energy_column_] = std::log1p(row[ecal_energy_column_]);
  }
  if (feature_layout_ == FeatureLayout::EcalTpad && apply_log1p_trigger_pad_pe_ &&
      row[tpad_pe_column_] != 0.0) {
    row[tpad_pe_column_] = std::log1p(row[tpad_pe_column_]);
  }

  for (std::size_t i = 0; i < feature_means_.size(); i++) {
    row[i] = (row[i] - feature_means_[i]) / feature_stds_[i];
  }
}

EcalHitOriginClassifier::DecodedOutput EcalHitOriginClassifier::decodeOutputs(
    const std::vector<ldmx::EcalHit>& hits, std::size_t num_tokens,
    const FloatArrays& outputs) const {
  if (outputs.size() != 1) {
    EXCEPTION_RAISE("InvalidONNXOutput", "Expected one logits output tensor.");
  }

  const std::size_t rows =
      output_includes_context_tokens_ ? num_tokens : hits.size();
  const auto& logits = outputs.front();
  if (logits.size() % rows != 0) {
    EXCEPTION_RAISE("InvalidONNXOutput",
                    "Logits size is not divisible by the expected number of rows.");
  }

  DecodedOutput decoded;
  decoded.num_classes = logits.size() / rows;
  decoded.classifications.reserve(hits.size());

  for (std::size_t i = 0; i < hits.size(); i++) {
    const auto [classification, confidence] =
        classify(logits.data() + i * decoded.num_classes, decoded.num_classes);

    ldmx::EcalHitClassification result;
    result.setID(hits[i].getID());
    result.setClassification(classification);
    result.setConfidence(confidence);
    decoded.classifications.push_back(result);
  }

  return decoded;
}

void EcalHitOriginClassifier::addTruth(
    const framework::Event& event, std::size_t model_class_count,
    std::vector<ldmx::EcalHitClassification>& classifications) const {
  const auto& sim_hits = event.getCollection<ldmx::SimCalorimeterHit>(
      sim_hit_collection_, sim_hit_pass_name_);

  std::map<int, HitTruth> truth_by_hit_id;
  // origin -> {sum energy, sum energy*y}
  std::map<int, std::pair<double, double>> origin_y_sums;

  for (const auto& sim_hit : sim_hits) {
    std::map<int, OriginContribution> contributions;
    for (unsigned int i = 0; i < sim_hit.getNumberOfContribs(); i++) {
      const auto contribution = sim_hit.getContrib(static_cast<int>(i));
      auto& origin = contributions[contribution.origin_id_];
      origin.energy += contribution.edep_;
      origin.count++;
    }

    truth_by_hit_id[sim_hit.getID()] =
        dominantOrigin(contributions, truth_energy_tie_relative_tolerance_);

    const auto position = sim_hit.getPosition();
    for (const auto& [origin_id, contribution] : contributions) {
      auto& sums = origin_y_sums[origin_id];
      sums.first += contribution.energy;
      sums.second += contribution.energy * position[1];
    }
  }

  const auto class_by_origin =
      truthClasses(origin_y_sums, expected_origin_ids_, model_class_count);

  for (auto& result : classifications) {
    const auto truth_it = truth_by_hit_id.find(result.getID());
    if (truth_it == truth_by_hit_id.end()) continue;

    result.setTruthOriginID(truth_it->second.origin_id);
    result.setTruthFraction(truth_it->second.fraction);

    const auto class_it = class_by_origin.find(truth_it->second.origin_id);
    if (class_it == class_by_origin.end()) continue;

    result.setTruthClassification(class_it->second);
    result.setHasTruth(true);
    result.setCorrectlyClassified(result.getClassification() == class_it->second);
  }
}

std::pair<int, float> EcalHitOriginClassifier::classify(
    const float* logits, std::size_t num_classes) {
  const auto max_iter = std::max_element(logits, logits + num_classes);
  const std::size_t max_index = std::distance(logits, max_iter);
  const double max_logit = *max_iter;

  double denominator= 0.;
  for (std::size_t i = 0; i < num_classes; i++) {
    denominator += std::exp(static_cast<double>(logits[i]) - max_logit);
  }

  return {static_cast<int>(max_index + 1),
          static_cast<float>(1.0 / denominator)};
}

}  // namespace ecal

DECLARE_PRODUCER(ecal::EcalHitOriginClassifier);
