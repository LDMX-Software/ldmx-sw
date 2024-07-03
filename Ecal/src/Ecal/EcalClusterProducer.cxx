/**
 * @file EcalClusterProducer.cxx
 * @brief Producer that performs clustering in the ECal
 * @author Josh Hiltbrand, University of Minnesota
 */

#include "Ecal/EcalClusterProducer.h"

namespace ecal {

EcalClusterProducer::EcalClusterProducer(const std::string& name,
                                         framework::Process& process)
    : Producer(name, process) {}

EcalClusterProducer::~EcalClusterProducer() {}

void EcalClusterProducer::configure(framework::config::Parameters& parameters) {
  cutoff_ = parameters.getParameter<double>("cutoff");
  seedThreshold_ = parameters.getParameter<double>("seedThreshold");
  growthThreshold_ = parameters.getParameter<double>("growthThreshold");
  cellFilter_ = parameters.getParameter<double>("cellFilter");
  digiCollName_ = parameters.getParameter<std::string>("digiCollName");
  digisPassName_ = parameters.getParameter<std::string>("digisPassName");
  recHitCollName_ = parameters.getParameter<std::string>("recHitCollName");
  algoCollName_ = parameters.getParameter<std::string>("algoCollName");
  algoName_ = parameters.getParameter<std::string>("algoName");
  clusterCollName_ = parameters.getParameter<std::string>("clusterCollName");
  old_ = parameters.getParameter<bool>("old");
  // fullHistory_ = parameters.getParameter<bool>("fullHistory");
}

void EcalClusterProducer::produce(framework::Event& event) {

  std::vector<ldmx::EcalHit> ecalHits =
      event.getCollection<ldmx::EcalHit>(recHitCollName_, digisPassName_);
  int nEcalDigis = ecalHits.size();

  // Don't do anything if there are no ECal digis!
  if (!(nEcalDigis > 0)) {
    return;
  }

  if (old_) {
    // Get the Ecal Geometry
    const auto& geometry = getCondition<ldmx::EcalGeometry>(
        ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

    TemplatedClusterFinder<MyClusterWeight> cf;

    for (ldmx::EcalHit& hit : ecalHits) {
      // Skip zero energy digis.
      if (hit.getEnergy() == 0) {
        continue;
      }

      cf.add(&hit, geometry);
    }

    cf.cluster(seedThreshold_, cutoff_);
    std::vector<WorkingCluster> wcVec = cf.getClusters();

    auto nLoops = cf.getNLoops();
    histograms_.fill("nLoops", nLoops);
    histograms_.fill("nClusters", wcVec.size());

    std::map<int, double> cWeights = cf.getWeights();

    ldmx::ClusterAlgoResult algoResult;
    algoResult.set(algoName_, 3, cWeights.rbegin()->first);
    algoResult.setAlgoVar(0, cutoff_);
    algoResult.setAlgoVar(1, seedThreshold_);
    algoResult.setAlgoVar(2, cf.getNSeeds());

    std::map<int, double>::iterator it = cWeights.begin();
    for (it = cWeights.begin(); it != cWeights.end(); it++) {
      algoResult.setWeight(it->first, it->second / 100);
      histograms_.fill("seed_weights", it->first, it->second);
    }

    std::vector<ldmx::EcalCluster> ecalClusters;
    for (int aWC = 0; aWC < wcVec.size(); aWC++) {
      ldmx::EcalCluster cluster;

      cluster.setEnergy(wcVec[aWC].centroid().E());
      cluster.setCentroidXYZ(wcVec[aWC].centroid().Px(),
                            wcVec[aWC].centroid().Py(),
                            wcVec[aWC].centroid().Pz());
      cluster.setNHits(wcVec[aWC].getHits().size());
      cluster.addHits(wcVec[aWC].getHits());

      histograms_.fill("nHits", wcVec[aWC].getHits().size());
      histograms_.fill("cluster_energy", wcVec[aWC].centroid().E());

      ecalClusters.push_back(cluster);
    }

    event.add(clusterCollName_, ecalClusters);
    event.add(algoCollName_, algoResult);
  } else {
    TemplatedEcalClusterFinder<EcalClusterWeight> cf;

    cf.cluster(ecalHits, seedThreshold_, growthThreshold_, cellFilter_, cutoff_);
    std::vector<WorkingEcalCluster> wcVec = cf.getClusters();

    auto nLoops = cf.getNLoops();
    histograms_.fill("nLoops", nLoops);
    histograms_.fill("nClusters", wcVec.size());

    const auto& cWeights = cf.getWeights();

    for (auto& [hitsLeft, weight]: cWeights) {
      histograms_.fill("seed_weights", hitsLeft, weight);
    }

    std::vector<ldmx::EcalCluster> ecalClusters;
    for (int aWC = 0; aWC < wcVec.size(); aWC++) {
      ldmx::EcalCluster cluster;

      cluster.setEnergy(wcVec[aWC].centroid().E());
      cluster.setCentroidXYZ(wcVec[aWC].centroid().Px(),
                            wcVec[aWC].centroid().Py(),
                            wcVec[aWC].centroid().Pz());
      cluster.setNHits(wcVec[aWC].getHits().size());
      cluster.addHits(wcVec[aWC].getHits());

      histograms_.fill("nHits", wcVec[aWC].getHits().size());
      histograms_.fill("cluster_energy", wcVec[aWC].centroid().E());

      ecalClusters.push_back(cluster);
    }

    event.add(clusterCollName_, ecalClusters);
  }
}
}  // namespace ecal

DECLARE_PRODUCER_NS(ecal, EcalClusterProducer);
