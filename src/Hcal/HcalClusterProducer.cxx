#include "Hcal/HcalClusterProducer.h"

#include <exception>
#include <iostream>

#include "Hcal/MyClusterWeight.h"
#include "Hcal/TemplatedClusterFinder.h"
#include "Hcal/WorkingCluster.h"
#include "TFile.h"
#include "TString.h"
#include "TTree.h"

namespace hcal {

HcalClusterProducer::HcalClusterProducer(const std::string& name,
                                         framework::Process& process)
    : Producer(name, process) {}

void HcalClusterProducer::configure(framework::config::Parameters& parameters) {
  EnoiseCut_ = parameters.getParameter<double>("EnoiseCut");
  deltaTime_ = parameters.getParameter<double>("deltaTime");
  deltaR_ = parameters.getParameter<double>("deltaR");
  EminCluster_ = parameters.getParameter<double>("EminCluster");
  cutOff_ = parameters.getParameter<double>("cutOff");

  clusterCollName_ = parameters.getParameter<std::string>("clusterCollName");
}

static bool compHitTimes(const ldmx::HcalHit* a, const ldmx::HcalHit* b) {
  return a->getTime() < b->getTime();
}

void HcalClusterProducer::produce(framework::Event& event) {
  const ldmx::HcalGeometry& hcalGeom = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);

  TemplatedClusterFinder<MyClusterWeight> finder;

  std::vector<ldmx::HcalCluster> hcalClusters;
  std::list<const ldmx::HcalHit*> seedList;
  std::vector<ldmx::HcalHit> hcalHits =
      event.getCollection<ldmx::HcalHit>("HcalRecHits");

  if (hcalHits.empty()) {
    return;
  }

  for (ldmx::HcalHit& hit : hcalHits) {
    if (hit.getEnergy() < EnoiseCut_) continue;
    if (hit.getEnergy() == 0) continue;
    finder.add(&hit, hcalGeom);
  }

  // seedList.sort([](const ldmx::HcalHit* a, const ldmx::HcalHit* b) {return
  // a->getEnergy() > b->getEnergy();});
  finder.cluster(EminCluster_, cutOff_, deltaTime_);

  std::vector<WorkingCluster> wcVec = finder.getClusters();
  for (unsigned int c = 0; c < wcVec.size(); c++) {
    if (wcVec[c].empty()) continue;
    ldmx::HcalCluster cluster;
    cluster.setEnergy(wcVec[c].centroid().E());
    cluster.setCentroidXYZ(wcVec[c].centroid().Px(), wcVec[c].centroid().Py(),
                           wcVec[c].centroid().Pz());
    cluster.setNHits(wcVec[c].getHits().size());
    cluster.addHits(wcVec[c].getHits());
    std::vector<const ldmx::HcalHit*> hits = wcVec[c].getHits();
    if (hits.size() > 0) {
      std::sort(hits.begin(), hits.end(), compHitTimes);
      cluster.setTime(hits[0]->getTime());
    }
    hcalClusters.push_back(cluster);
  }
  event.add(clusterCollName_, hcalClusters);
}

}  // namespace hcal
DECLARE_PRODUCER_NS(hcal, HcalClusterProducer);
