#include "DQM/EcalHitOriginClassifierResults.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <iterator>
#include <map>
#include <string>
#include <unordered_map>

#include "DetDescr/EcalID.h"
#include "Ecal/Event/EcalHit.h"
#include "Ecal/Event/EcalHitClassification.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "SimCore/Event/SimTrackerHit.h"

namespace {

  // this helps with repetitive "cluster moment" (even if hit grouping is not
  // from classical clustering) calculations 
struct CentroidAccumulator {
  double weight= 0.;
  double x= 0.;
  double y= 0.;
  double z= 0.;
  double r2_xy= 0.;
  double r2_xyz= 0.;
  void add(double x_value, double y_value, double z_value,
           double sample_weight) {
    weight += sample_weight;
    x += sample_weight * x_value;
    y += sample_weight * y_value;
    z += sample_weight * z_value;
    r2_xy += sample_weight * (x_value * x_value + y_value * y_value);
    r2_xyz += sample_weight *
              (x_value * x_value + y_value * y_value + z_value * z_value);
  }

  double centroidX() const { return x / weight; }
  double centroidY() const { return y / weight; }
  double centroidZ() const { return z / weight; }

  double radialWidth() const {
    double cx = centroidX();
    double cy = centroidY();
    return std::sqrt(std::max(0.0, r2_xy / weight - cx * cx - cy * cy));
  }

  // a generalization of width, probably not actually useful 
  double spatialWidth() const {
    double cx = centroidX();
    double cy = centroidY();
    double cz = centroidZ();
    return std::sqrt(
        std::max(0.0, r2_xyz / weight - cx * cx - cy * cy - cz * cz));
  }
};

struct Separation {
  double min_xy{std::numeric_limits<double>::max()};
  double mean_xy{0.0};
  int pairs{0};
};

Separation calculateSeparation(
    const std::map<int, CentroidAccumulator>& origins) {
  Separation result;

  // loop over all possible pairs
  for (auto i = origins.begin(); i != origins.end(); i++) {
    for (auto j = std::next(i); j != origins.end(); j++) {
      double dx = i->second.centroidX() - j->second.centroidX();
      double dy = i->second.centroidY() - j->second.centroidY();
      double distance_xy = std::hypot(dx, dy);
      result.min_xy = std::min(result.min_xy, distance_xy);
      result.mean_xy += distance_xy;
      result.pairs++;
    }
  }

  if (result.pairs > 0) {
    result.mean_xy /= result.pairs;
  }
  return result;
}

//this extracts the encoded originID from the OverlayProducer
int originIDFromTrackID(int track_id) {
  return ((static_cast<unsigned int>(track_id) >> 24) & 0x7u) + 1;
}

}  // namespace

namespace dqm {

void EcalHitOriginClassifierResults::configure(
    framework::config::Parameters& parameters) {
  hit_collection_ = parameters.get<std::string>("hit_collection");
  hit_pass_name_ = parameters.get<std::string>("hit_pass_name");
  classification_collection_ =
      parameters.get<std::string>("classification_collection");
  classification_pass_name_ =
      parameters.get<std::string>("classification_pass_name");
  sim_hit_collection_ = parameters.get<std::string>("sim_hit_collection");
  sim_hit_pass_name_ = parameters.get<std::string>("sim_hit_pass_name");
  scoring_plane_collection_ =
      parameters.get<std::string>("scoring_plane_collection");
  scoring_plane_pass_name_ =
      parameters.get<std::string>("scoring_plane_pass_name");
}

void EcalHitOriginClassifierResults::analyze(const framework::Event& event) {
  const auto& hits =
      event.getCollection<ldmx::EcalHit>(hit_collection_, hit_pass_name_);
  const auto& classifications =
      event.getCollection<ldmx::EcalHitClassification>(
          classification_collection_, classification_pass_name_);
  const auto& sim_hits = event.getCollection<ldmx::SimCalorimeterHit>(
      sim_hit_collection_, sim_hit_pass_name_);
  const auto& scoring_hits = event.getCollection<ldmx::SimTrackerHit>(
      scoring_plane_collection_, scoring_plane_pass_name_);

  std::unordered_map<int, const ldmx::EcalHit*> hit_by_id;
  for (const auto& hit : hits) hit_by_id[hit.getID()] = &hit;

  int truth_hits= 0;
  int correct_hits= 0;
  double total_energy= 0.;
  double correct_energy = 0.;
  
  for (const auto& result : classifications) {
    histograms_.fill("classification", result.getClassification());
    histograms_.fill("confidence", result.getConfidence());
    histograms_.fill("has_truth", result.hasTruth());
    histograms_.fill("truth_origin_id", result.getTruthOriginID());

    if (!result.hasTruth())
      continue;
    truth_hits++;
    if (result.isCorrectlyClassified()) 
      correct_hits++;
    histograms_.fill("truth_classification", result.getTruthClassification());
    histograms_.fill("truth_fraction", result.getTruthFraction());
    histograms_.fill("correct", result.isCorrectlyClassified());
    histograms_.fill("truth_classification:classification",
                     result.getTruthClassification(),
                     result.getClassification());
    histograms_.fill("confidence:correct", result.getConfidence(),
                     result.isCorrectlyClassified());

    const auto hit = hit_by_id.find(result.getID());
    if (hit != hit_by_id.end()) {
      double hit_energy = hit->second->getEnergy();
      total_energy += hit_energy;
      if (result.isCorrectlyClassified())
	correct_energy += hit_energy;
      histograms_.fill("hit_energy:correct", hit_energy,
                       result.isCorrectlyClassified());
      histograms_.fill("layer:correct", ldmx::EcalID(result.getID()).layer(),
                       result.isCorrectlyClassified());
    }
  }

  histograms_.fill("num_hits", hits.size());
  histograms_.fill("num_classifications", classifications.size());
  histograms_.fill("num_truth_hits", truth_hits);
  histograms_.fill("num_correct_hits", correct_hits);

  bool has_accuracy = truth_hits > 0; // ? true : false;
  double event_accuracy =
    has_accuracy ? double(correct_hits)/truth_hits : 1.0;
  if (has_accuracy) {
    histograms_.fill("event_accuracy", event_accuracy);
    // since accuracy is yes/no, energy weighted accuracy reduces to
    // correct energy fraction
    histograms_.fill("event_energy_weighted_accuracy",
                     correct_energy / total_energy);
  }

  //look at separation as measured in various ecal layer windows 
  std::vector<int> max_layers{-1, 1, 20};
  std::vector<const char*> layer_names{"all", "first1", "first20"};
  std::array<std::map<int, CentroidAccumulator>, 3> unweighted;
  std::array<std::map<int, CentroidAccumulator>, 3> energy_weighted;

  for (const auto& sim_hit : sim_hits) {
    std::map<int, double> energy_by_origin;
    for (unsigned int i = 0; i < sim_hit.getNumberOfContribs(); i++) {
      const auto contribution = sim_hit.getContrib(static_cast<int>(i));
      energy_by_origin[contribution.origin_id_] += contribution.edep_;
      histograms_.fill("sim_origin_id", contribution.origin_id_);
    }

    int layer = ldmx::EcalID(sim_hit.getID()).layer();
    const auto pos = sim_hit.getPosition();
    for (const auto& [or_id, energy] : energy_by_origin) {
      for (std::size_t window = 0; window < max_layers.size(); window++) {
        if (max_layers[window] >= 0 && layer >= max_layers[window]) continue;
        unweighted[window][or_id].add(pos[0], pos[1], pos[2], 1.0);
        energy_weighted[window][or_id].add(pos[0], pos[1], pos[2], energy);
      }
    }
  }
  
  // define a lambda function for repetitive histogram filling
  const auto fill_geometry =
      [this, has_accuracy, event_accuracy](
          const std::map<int, CentroidAccumulator>& origins,
          const std::string& name) {
        for (const auto& [or_id, centroid] : origins) {
          histograms_.fill("centroid_x_" + name, centroid.centroidX());
          histograms_.fill("centroid_y_" + name, centroid.centroidY());
          histograms_.fill("centroid_z_" + name, centroid.centroidZ());
          histograms_.fill("radial_width_" + name, centroid.radialWidth());
        } // over origin IDs and centroids

        const auto separation = calculateSeparation(origins);
        if (separation.pairs == 0) return;

        histograms_.fill("min_separation_" + name, separation.min_xy);
        histograms_.fill("mean_separation_" + name, separation.mean_xy);
        if (has_accuracy) {
          histograms_.fill("min_separation_" + name + ":event_accuracy",
                           separation.min_xy, event_accuracy);
        }
      };//histogram filling lambda def

  for (std::size_t window = 0; window < layer_names.size(); window++) {
    fill_geometry(unweighted[window],
                  std::string{"sim_unweighted_"} + layer_names[window]);
    fill_geometry(energy_weighted[window],
                  std::string{"sim_weighted_"} + layer_names[window]);
  }

  std::map<int, CentroidAccumulator> scoring_plane;
  for (const auto& hit : scoring_hits) {
    //only keep forward-going, electron, Ecal face scoring plane hits
    if (hit.getPdgID() != 11 || hit.getMomentum()[2] <= 0 || fabs(hit.getPosition()[2] - 240) > 0.1 ) continue;
    const int or_id = originIDFromTrackID(hit.getTrackID());
    const auto pos = hit.getPosition();
    scoring_plane[or_id].add(pos[0], pos[1], pos[2], 1.0);
  }
  fill_geometry(scoring_plane, "scoring_plane");

  /* TODO
     somewhere here, add looking at accuracy vs angle of incidence, calculated from the SP momentum.
     ideally this would even be average event accuracy vs  separation AND angle, to show effects of overlap
     that varies with Ecal layer.
     the easiest way to accomplish this is usually to fill one 2D histogram with the (energy weighted or not)
     event accuracy, and one with just increments of 1 at the same 2D bin, to keep track of the number of
     events in each bin.
     then at the end of processing one divides the former by the latter to get the average.
     since ldmx-sw uses its own framework approach to ROOT histogram wrangling rather than exposing the
     ROOT methods, this is not as straightforward as a ROOT TH1::Divide(), but that can be done in plotting
     code later. 
  */
  
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::EcalHitOriginClassifierResults);
