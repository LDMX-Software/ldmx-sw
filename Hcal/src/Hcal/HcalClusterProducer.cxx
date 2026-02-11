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
  enoise_cut_ = parameters.get<double>("enoise_cut");
  delta_time_ = parameters.get<double>("delta_time");
  delta_r_ = parameters.get<double>("delta_r");
  emin_cluster_ = parameters.get<double>("emin_cluster");
  cut_off_ = parameters.get<double>("cut_off");

  cluster_coll_name_ = parameters.get<std::string>("cluster_coll_name");
  hcal_hits_pass_name_ = parameters.get<std::string>("hcal_hits_pass_name");
}

static bool compHitTimes(const ldmx::HcalHit* a, const ldmx::HcalHit* b) {
  return a->getTime() < b->getTime();
}

void HcalClusterProducer::produce(framework::Event& event) {
  const ldmx::HcalGeometry& hcal_geom = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);

  TemplatedClusterFinder<MyClusterWeight> finder;

  std::vector<ldmx::HcalCluster> hcal_clusters;
  std::list<const ldmx::HcalHit*> seed_list;
  std::vector<ldmx::HcalHit> hcal_hits =
      event.getCollection<ldmx::HcalHit>("HcalRecHits", hcal_hits_pass_name_);

  if (hcal_hits.empty()) {
    return;
  }

  for (ldmx::HcalHit& hit : hcal_hits) {
    if (hit.getEnergy() < enoise_cut_) continue;
    if (hit.getEnergy() == 0) continue;
    finder.add(&hit, hcal_geom);
  }

  // seedList.sort([](const ldmx::HcalHit* a, const ldmx::HcalHit* b) {return
  // a->getEnergy() > b->getEnergy();});
  finder.cluster(emin_cluster_, cut_off_, delta_time_);

  std::vector<WorkingCluster> wc_vec = finder.getClusters();
  for (unsigned int c = 0; c < wc_vec.size(); c++) {
    if (wc_vec[c].empty()) continue;
    ldmx::HcalCluster cluster;
    cluster.setEnergy(wc_vec[c].centroid().E());
    cluster.setCentroidXYZ(wc_vec[c].centroid().Px(), wc_vec[c].centroid().Py(),
                           wc_vec[c].centroid().Pz());
    cluster.setNHits(wc_vec[c].getHits().size());
    cluster.addHits(wc_vec[c].getHits());
    std::vector<const ldmx::HcalHit*> hits = wc_vec[c].getHits();
    if (hits.size() > 0) {
      std::sort(hits.begin(), hits.end(), compHitTimes);
      cluster.setTime(hits[0]->getTime());
    }
    hcal_clusters.push_back(cluster);
  }
  event.add(cluster_coll_name_, hcal_clusters);
}

}  // namespace hcal
DECLARE_PRODUCER(hcal::HcalClusterProducer);
