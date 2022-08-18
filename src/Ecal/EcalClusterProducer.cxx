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
  digisPassName_ = parameters.getParameter<std::string>("digisPassName");
  algoCollName_ = parameters.getParameter<std::string>("algoCollName");
  algoName_ = parameters.getParameter<std::string>("algoName");
  clusterCollName_ = parameters.getParameter<std::string>("clusterCollName");
}

void EcalClusterProducer::produce(framework::Event& event) {
  // Get the Ecal Geometry
  const auto& geometry = getCondition<ldmx::EcalGeometry>(
      ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  TemplatedClusterFinder<MyClusterWeight> cf;

  std::vector<ldmx::EcalHit> ecalHits =
      event.getCollection<ldmx::EcalHit>("ecalDigis", digisPassName_);
  int nEcalDigis = ecalHits.size();

  // Don't do anything if there are no ECal digis!
  if (!(nEcalDigis > 0)) {
    return;
  }

  for (ldmx::EcalHit& hit : ecalHits) {
    // Skip zero energy digis.
    if (hit.getEnergy() == 0) {
      continue;
    }

    cf.add(&hit, geometry);
  }

  cf.cluster(seedThreshold_, cutoff_);
  std::vector<WorkingCluster> wcVec = cf.getClusters();

  std::map<int, double> cWeights = cf.getWeights();

  ldmx::ClusterAlgoResult algoResult;
  algoResult.set(algoName_, 3, cWeights.rbegin()->first);
  algoResult.setAlgoVar(0, cutoff_);
  algoResult.setAlgoVar(1, seedThreshold_);
  algoResult.setAlgoVar(2, cf.getNSeeds());

  std::map<int, double>::iterator it = cWeights.begin();
  for (it = cWeights.begin(); it != cWeights.end(); it++) {
    algoResult.setWeight(it->first, it->second / 100);
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

    ecalClusters.push_back(cluster);
  }

  event.add(clusterCollName_, ecalClusters);
  event.add(algoCollName_, algoResult);
}
}  // namespace ecal

DECLARE_PRODUCER_NS(ecal, EcalClusterProducer);
