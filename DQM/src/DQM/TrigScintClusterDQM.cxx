#include "DQM/TrigScintClusterDQM.h"

namespace dqm {

TrigScintClusterDQM::TrigScintClusterDQM(const std::string& name,
                                         framework::Process& process)
    : framework::Analyzer(name, process) {}

void TrigScintClusterDQM::onProcessStart() {
  getHistoDirectory();

  histograms_.create("centroid", "Cluster channel centroid", 500, 0, 100);
  histograms_.create("total_pe", "Total cluster PEs in the pad/event", 500, 0,
                     2000);
  histograms_.create("n_clusters", "Clusters in the pad/event", 25, 0, 25);
  histograms_.create("n_hits", "n_{hits} forming the clusters", 4, 0, 4);
  histograms_.create("seed", "Cluster seed hit channel ID", 100, 0, 100);
  histograms_.create("beamEfrac", "Cluster edep fraction from beam electron",
                     101, 0., 1.01);
  histograms_.create("x", "Cluster x position [mm]", 1000, -100, 100);
  histograms_.create("y", "Cluster y position [mm]", 1000, -100, 100);
  histograms_.create("z", "Cluster z position [mm]", 1000, -900, 100);

  histograms_.create("pe", "PE in a cluster", 250, 0, 1000);
  histograms_.create("energy", "Energy dep in a cluster [MeV]", 500, 0, 1500);
  histograms_.create("cluster_time", "Cluster time [ns]", 600, -150, 150);

  // TODO: implement getting a list of the constructed histograms, to iterate
  // through and set overflow boolean.
}

void TrigScintClusterDQM::configure(framework::config::Parameters& ps) {
  cluster_collection_name_ = ps.get<std::string>("cluster_collection");
  pad_name_ = ps.get<std::string>("pad").c_str();
  pass_name_ = ps.get<std::string>("pass_name").c_str();

  ldmx_log(debug) << "Collection name =  " << cluster_collection_name_
                  << ", pad = " << pad_name_ << ", pass = " << pass_name_;
}

void TrigScintClusterDQM::analyze(const framework::Event& event) {
  if (not event.exists(cluster_collection_name_, pass_name_)) return;
  // Get the collection of TrigScintCluster digitized clusters if the exists
  const std::vector<ldmx::TrigScintCluster> trig_scint_clusters =
      event.getCollection<ldmx::TrigScintCluster>(cluster_collection_name_,
                                                  pass_name_);

  double total_pe{0};
  // Loop through all TrigScint clusters in the event
  for (const ldmx::TrigScintCluster& cluster : trig_scint_clusters) {
    histograms_.fill("pe", cluster.getPE());
    histograms_.fill("energy", cluster.getEnergy());
    histograms_.fill("cluster_time", cluster.getTime());
    histograms_.fill("centroid", cluster.getCentroid());
    histograms_.fill("n_hits", cluster.getNHits());
    histograms_.fill("seed", cluster.getSeed());
    histograms_.fill("beamEfrac", cluster.getBeamEfrac());

    histograms_.fill("x", cluster.getCentroidX());
    histograms_.fill("y", cluster.getCentroidY());
    histograms_.fill("z", cluster.getCentroidZ());

    total_pe += cluster.getPE();
  }

  histograms_.fill("n_clusters", trig_scint_clusters.size());
  histograms_.fill("total_pe", total_pe);
}

}  // namespace dqm

DECLARE_ANALYZER(dqm::TrigScintClusterDQM)
