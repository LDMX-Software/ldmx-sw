#include "TrigScint/ClusterTripletMaker.h"

#include <algorithm>

namespace trigscint {

ClusterTripletMaker::ClusterTripletMaker(const std::string& name,
                                         framework::Process& process)
    : Analyzer(name, process) {}

void ClusterTripletMaker::configure(framework::config::Parameters& ps) {
  pass_name_ = ps.get<std::string>("pass_name");
  cluster_input_collections_ =
      ps.get<std::vector<std::string>>("cluster_input_collections");
  output_collection_ = ps.get<std::string>("output_collection");
  verbose_ = ps.get<int>("verbosity");

  if (verbose_) {
    ldmx_log(info) << "In ClusterTripletMaker: configure done!"
                   << "\nInput collection 1: "
                   << cluster_input_collections_.at(0)
                   << "\nInput collection 2: "
                   << cluster_input_collections_.at(1)
                   << "\nInput collection 3: "
                   << cluster_input_collections_.at(2)
                   << "\nPass name: " << pass_name_
                   << "\nOutput file: " << output_collection_
                   << "\nVerbosity: " << verbose_;
  }

  return;
}

void ClusterTripletMaker::onProcessStart() {
  output_stream_.open(output_collection_);

  if (verbose_) {
    ldmx_log(info) << "Opened output file " << output_collection_;
  }

  return;
}

void ClusterTripletMaker::analyze(const framework::Event& event) {
  const auto clusters_pad1{event.getCollection<ldmx::TrigScintCluster>(
      cluster_input_collections_.at(0), pass_name_)};

  const auto clusters_pad2{event.getCollection<ldmx::TrigScintCluster>(
      cluster_input_collections_.at(1), pass_name_)};

  const auto clusters_pad3{event.getCollection<ldmx::TrigScintCluster>(
      cluster_input_collections_.at(2), pass_name_)};

  const size_t num_clusters = std::min(
      {clusters_pad1.size(), clusters_pad2.size(), clusters_pad3.size()});

  if (verbose_ > 1) {
    ldmx_log(debug) << "Event " << event.getEventNumber() << " has "
                    << clusters_pad1.size() << ", " << clusters_pad2.size()
                    << ", " << clusters_pad3.size()
                    << " clusters in pads 1, 2, and 3.";
  }

  for (size_t cluster_index = 0; cluster_index < num_clusters;
       ++cluster_index) {
    const int event_number = event.getEventNumber();

    const float pad1_centroid = clusters_pad1.at(cluster_index).getCentroid();
    const float pad2_centroid = clusters_pad2.at(cluster_index).getCentroid();
    const float pad3_centroid = clusters_pad3.at(cluster_index).getCentroid();

    output_stream_ << event_number << " " << pad1_centroid << " "
                   << pad2_centroid << " " << pad3_centroid << " " << "\n";
  }

  if (verbose_) {
    for (size_t cluster_index = 1; cluster_index < clusters_pad1.size();
         ++cluster_index) {
      ldmx_log(info) << "Event " << event.getEventNumber()
                     << ", extra cluster in pad 1: "
                     << clusters_pad1.at(cluster_index).getCentroid();
    }

    for (size_t cluster_index = 1; cluster_index < clusters_pad2.size();
         ++cluster_index) {
      ldmx_log(info) << "Event " << event.getEventNumber()
                     << ", extra cluster in pad 2: "
                     << clusters_pad2.at(cluster_index).getCentroid();
    }

    for (size_t cluster_index = 1; cluster_index < clusters_pad3.size();
         ++cluster_index) {
      ldmx_log(info) << "Event " << event.getEventNumber()
                     << ", extra cluster in pad 3: "
                     << clusters_pad3.at(cluster_index).getCentroid();
    }
  }

  return;
}

void ClusterTripletMaker::onProcessEnd() {
  if (output_stream_.is_open()) {
    output_stream_.close();
  }

  if (verbose_) {
    ldmx_log(info) << "Closed output file " << output_collection_;
  }

  return;
}

}  // namespace trigscint

DECLARE_ANALYZER(trigscint::ClusterTripletMaker);
