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
    std::vector<IntermediateCluster> wc_vec = cf.getClusters();
    std::vector<IntermediateCluster> f_wc_vec = cf.getFirstLayerCentroids();

    auto n_loops = cf.getNLoops();
    histograms_.fill("nLoops", n_loops);
    histograms_.fill("nClusters", wc_vec.size());
    if (reclustering_)
      histograms_.fill("recluster", cf.getInitialClusterNbr(), wc_vec.size());

    std::vector<ldmx::EcalCluster> ecal_clusters;
    for (int a_wc = 0; a_wc < wc_vec.size(); a_wc++) {
      ldmx::EcalCluster cluster;

      cluster.setEnergy(wc_vec[a_wc].centroid().E());
      cluster.setCentroidXYZ(wc_vec[a_wc].centroid().Px(),
                             wc_vec[a_wc].centroid().Py(),
                             wc_vec[a_wc].centroid().Pz());
      cluster.setFirstLayerCentroidXYZ(f_wc_vec[a_wc].centroid().Px(),
                                       f_wc_vec[a_wc].centroid().Py(),
                                       f_wc_vec[a_wc].centroid().Pz());
      cluster.setNHits(wc_vec[a_wc].getHits().size());
      cluster.addHits(wc_vec[a_wc].getHits());
      cluster.addFirstLayerHits(f_wc_vec[a_wc].getHits());

      float cl_x(0), cl_y(0), cl_z(0), cl_xx(0), cl_yy(0), cl_zz(0);
      float cl_w = 1;  // weight
      float sumw = 0;

      for (auto hit : wc_vec[a_wc].getHits()) {
        if (hit->getEnergy() < minHitEnergy_) continue;
        cl_w = log(hit->getEnergy()) - log(minHitEnergy_);
        cl_x += cl_w * hit->getXPos();
        cl_y += cl_w * hit->getYPos();
        cl_z += cl_w * hit->getZPos();
        cl_xx += cl_w * hit->getXPos() * hit->getXPos();
        cl_yy += cl_w * hit->getYPos() * hit->getYPos();
        cl_zz += cl_w * hit->getZPos() * hit->getZPos();
        sumw += cl_w;
      }  // over hits
      // could probably get this as cluster.getCentroidX() instead
      cl_x /= sumw;  // now is <x>
      cl_y /= sumw;
      cl_z /= sumw;
      cl_xx /= sumw;  // now is <x^2>
      cl_yy /= sumw;
      cl_zz /= sumw;
      cl_xx = sqrt(cl_xx - cl_x * cl_x);  // now is sqrt(<x^2>-<x>^2)
      cl_yy = sqrt(cl_yy - cl_y * cl_y);
      cl_zz = sqrt(cl_zz - cl_z * cl_z);

      cluster.setRMSXYZ(cl_xx, cl_yy, cl_zz);

      histograms_.fill("nHits", wc_vec[a_wc].getHits().size());
      histograms_.fill("cluster_energy", wc_vec[a_wc].centroid().E());

      ecal_clusters.push_back(cluster);
    }

    event.add(cluster_coll_name_, ecal_clusters);
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
    std::vector<IntermediateCluster> wc_vec = cf.getClusters();

    auto n_loops = cf.getNLoops();
    histograms_.fill("nLoops", n_loops);
    histograms_.fill("nClusters", wc_vec.size());

    std::map<int, double> c_weights = cf.getWeights();

    ldmx::ClusterAlgoResult algo_result;
    algo_result.set(algo_name_, 3, c_weights.rbegin()->first);
    algo_result.setAlgoVar(0, cutoff_);
    algo_result.setAlgoVar(1, seed_threshold_);
    algo_result.setAlgoVar(2, cf.getNSeeds());

    std::map<int, double>::iterator it = c_weights.begin();
    for (it = c_weights.begin(); it != c_weights.end(); it++) {
      algo_result.setWeight(it->first, it->second / 100);
      histograms_.fill("seed_weights", it->first, it->second);
    }

    std::vector<ldmx::EcalCluster> ecal_clusters;
    for (int a_wc = 0; a_wc < wc_vec.size(); a_wc++) {
      ldmx::EcalCluster cluster;

      cluster.setEnergy(wc_vec[a_wc].centroid().E());
      cluster.setCentroidXYZ(wc_vec[a_wc].centroid().Px(),
                             wc_vec[a_wc].centroid().Py(),
                             wc_vec[a_wc].centroid().Pz());
      cluster.setNHits(wc_vec[a_wc].getHits().size());
      cluster.addHits(wc_vec[a_wc].getHits());

      histograms_.fill("nHits", wc_vec[a_wc].getHits().size());
      histograms_.fill("cluster_energy", wc_vec[a_wc].centroid().E());

      ecal_clusters.push_back(cluster);
    }

    event.add(cluster_coll_name_, ecal_clusters);
    event.add(algo_coll_name_, algo_result);
  }
}
}  // namespace ecal

DECLARE_PRODUCER(ecal::EcalClusterProducer);
