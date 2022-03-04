#include "DQM/TrigScintClusterDQM.h"

namespace dqm {

TrigScintClusterDQM::TrigScintClusterDQM(const std::string &name,
                                         framework::Process &process)
    : framework::Analyzer(name, process) {}

TrigScintClusterDQM::~TrigScintClusterDQM() {}

void TrigScintClusterDQM::onProcessStart() {
  getHistoDirectory();

  histograms_.create("centroid", "Cluster channel centroid", 500, 0, 100);
  histograms_.create("total_pe", "Total cluster PEs in the pad/event", 500, 0,
                     2000);
  histograms_.create("n_clusters", "Clusters in the pad/event", 25, 0, 25);
  histograms_.create("n_hits", "N_{hits} forming the clusters", 4, 0, 4);
  histograms_.create("seed", "Cluster seed hit channel ID", 100, 0, 100);
  histograms_.create("beamEfrac", "Cluster edep fraction from beam electron",
                     101, 0., 1.01);
  histograms_.create("x", "Cluster x position", 1000, -100, 100);
  histograms_.create("y", "Cluster y position", 1000, -100, 100);
  histograms_.create("z", "Cluster z position", 1000, -900, 100);

  histograms_.create("pe", "PE in a cluster", 250, 0, 1000);
  histograms_.create("energy", "Energy dep in a cluster [MeV]", 500, 0, 1500);
  histograms_.create("cluster_time", "Cluster time (ns)", 600, -150, 150);

  // TODO: implement getting a list of the constructed histograms, to iterate
  // through and set overflow boolean.
}

void TrigScintClusterDQM::configure(framework::config::Parameters &ps) {
  clusterCollectionName_ = ps.getParameter<std::string>("cluster_collection");
  padName_ = ps.getParameter<std::string>("pad").c_str();
  passName_ = ps.getParameter<std::string>("passName").c_str();

  ldmx_log(info) << "In TrigScintClusterDQM::configure, got parameters "
                 << clusterCollectionName_ << ", pad = " << padName_
                 << ", pass = " << passName_;
}

void TrigScintClusterDQM::analyze(const framework::Event &event) {
  // Get the collection of TrigScintCluster digitized clusters if the exists
  const std::vector<ldmx::TrigScintCluster> TrigScintClusters =
      event.getCollection<ldmx::TrigScintCluster>(clusterCollectionName_,
                                                  passName_);

  double totalPE{0};
  // Loop through all TrigScint clusters in the event
  for (const ldmx::TrigScintCluster &cluster : TrigScintClusters) {
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

    totalPE += cluster.getPE();
  }

  histograms_.fill("n_clusters", TrigScintClusters.size());
  histograms_.fill("total_pe", totalPE);
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, TrigScintClusterDQM)
