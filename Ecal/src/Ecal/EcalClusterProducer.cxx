/**
 * @file EcalClusterProducer.cxx
 * @brief Producer that performs clustering in the ECal
 * @author Josh Hiltbrand, University of Minnesota
 */

#include "Ecal/EcalClusterProducer.h"

#include <iostream>

namespace ecal {

EcalClusterProducer::EcalClusterProducer(const std::string& name,
                                         framework::Process& process)
    : Producer(name, process) {}

void EcalClusterProducer::configure(framework::config::Parameters& parameters) {
  cutoff_ = parameters.getParameter<double>("cutoff");
  seed_threshold_ = parameters.getParameter<double>("seed_threshold");

  dc_ = parameters.getParameter<double>("dc");
  rhoc_ = parameters.getParameter<double>("rhoc");
  deltac_ = parameters.getParameter<double>("deltac");
  deltao_ = parameters.getParameter<double>("deltao");

  rec_hit_coll_name_ =
      parameters.getParameter<std::string>("rec_hit_coll_name");
  rec_hit_pass_name_ =
      parameters.getParameter<std::string>("rec_hit_pass_name");
  algo_coll_name_ = parameters.getParameter<std::string>("algo_coll_name");
  algo_name_ = parameters.getParameter<std::string>("algo_name");
  cluster_coll_name_ =
      parameters.getParameter<std::string>("cluster_coll_name");
  CLUE_ = parameters.getParameter<bool>("CLUE");
  nbr_of_layers_ = parameters.getParameter<int>("nbr_of_layers");
  reclustering_ = parameters.getParameter<bool>("reclustering");
}

void EcalClusterProducer::produce(framework::Event& event) {
  const auto& ecal_hits{event.getCollection<ldmx::EcalHit>(rec_hit_coll_name_,
                                                           rec_hit_pass_name_)};
  if (ecal_hits.size() == 0) {
    // don't do anything if there are no ECal hits
    return;
  }

  if (CLUE_) {
    CLUE cf;
    cf.cluster(ecal_hits, dc_, rhoc_, deltac_, deltao_, nbr_of_layers_,
               reclustering_);
    std::vector<IntermediateCluster> wcVec = cf.getClusters();
    std::vector<IntermediateCluster> fWcVec = cf.getFirstLayerCentroids();

    auto nLoops = cf.getNLoops();
    histograms_.fill("nLoops", nLoops);
    histograms_.fill("nClusters", wcVec.size());
    if (reclustering_)
      histograms_.fill("recluster", cf.getInitialClusterNbr(), wcVec.size());

    std::vector<ldmx::EcalCluster> ecalClusters;
    for (int aWC = 0; aWC < wcVec.size(); aWC++) {
      ldmx::EcalCluster cluster;

      cluster.setEnergy(wcVec[aWC].centroid().E());
      cluster.setCentroidXYZ(wcVec[aWC].centroid().Px(),
                             wcVec[aWC].centroid().Py(),
                             wcVec[aWC].centroid().Pz());
      cluster.setFirstLayerCentroidXYZ(fWcVec[aWC].centroid().Px(),
                                       fWcVec[aWC].centroid().Py(),
                                       fWcVec[aWC].centroid().Pz());
      cluster.setNHits(wcVec[aWC].getHits().size());
      cluster.addHits(wcVec[aWC].getHits());
      cluster.addFirstLayerHits(fWcVec[aWC].getHits());

      float x(0), y(0), z(0), xx(0), yy(0), zz(0), n(0);
      float w = 1;  // weight
      float sumw = 0;

      for (auto hit : wcVec[aWC].getHits()) {
        if (hit->getEnergy() < minHitEnergy_) continue;
        w = log(hit->getEnergy() - log(minHitEnergy_));
        x += w * hit->getXPos();
        y += w * hit->getYPos();
        z += w * hit->getZPos();
        xx += w * hit->getXPos() * hit->getXPos();
        yy += w * hit->getYPos() * hit->getYPos();
        zz += w * hit->getZPos() * hit->getZPos();
        n += 1;
        sumw += w;
      }  // over hits
      // could probably get this as cluster.getCentroidX() instead
      x /= sumw;  // now is <x>
      y /= sumw;
      z /= sumw;
      xx /= sumw;  // now is <x^2>
      yy /= sumw;
      zz /= sumw;
      xx = sqrt(xx - x * x);  // now is sqrt(<x^2>-<x>^2)
      yy = sqrt(yy - y * y);
      zz = sqrt(zz - z * z);

      cluster.setRMSXYZ(xx, yy, zz);

      histograms_.fill("nHits", wcVec[aWC].getHits().size());
      histograms_.fill("cluster_energy", wcVec[aWC].centroid().E());

      ecalClusters.push_back(cluster);
    }

    event.add(cluster_coll_name_, ecalClusters);
  } else {
    TemplatedClusterFinder<MyClusterWeight> cf;

    for (const ldmx::EcalHit& hit : ecal_hits) {
      // Skip zero energy digis.
      if (hit.getEnergy() == 0) {
        continue;
      }
      cf.add(hit);
    }

    cf.cluster(seed_threshold_, cutoff_);
    std::vector<IntermediateCluster> wcVec = cf.getClusters();

    auto nLoops = cf.getNLoops();
    histograms_.fill("nLoops", nLoops);
    histograms_.fill("nClusters", wcVec.size());

    std::map<int, double> cWeights = cf.getWeights();

    ldmx::ClusterAlgoResult algoResult;
    algoResult.set(algo_name_, 3, cWeights.rbegin()->first);
    algoResult.setAlgoVar(0, cutoff_);
    algoResult.setAlgoVar(1, seed_threshold_);
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

    event.add(cluster_coll_name_, ecalClusters);
    event.add(algo_coll_name_, algoResult);
  }
}
}  // namespace ecal

DECLARE_PRODUCER(ecal::EcalClusterProducer);
