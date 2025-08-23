#include "Recon/PFEcalClusterProducer.h"

#include "Recon/DBScanClusterBuilder.h"
#include "Recon/Event/CaloCluster.h"
#include "Recon/Event/CalorimeterHit.h"

namespace recon {

void PFEcalClusterProducer::configure(framework::config::Parameters& ps) {
  hit_coll_name_ = ps.get<std::string>("hitCollName");
  hit_pass_name_ = ps.get<std::string>("hitPassName");
  cluster_coll_name_ = ps.get<std::string>("clusterCollName");
  suffix_ = ps.get<std::string>("suffix", "");
  single_cluster_ = ps.get<bool>("doSingleCluster");
  log_energy_weight_ = ps.get<bool>("logEnergyWeight");
  // DBScan parameters
  min_cluster_hit_mult_ = ps.get<int>("minClusterHitMult");
  cluster_hit_dist_ = ps.get<double>("clusterHitDist");
  cluster_z_bias_ = ps.get<double>("clusterZBias", 1);
  min_hit_energy_ = ps.get<double>("minHitEnergy");
}

void PFEcalClusterProducer::produce(framework::Event& event) {
  if (!event.exists(hit_coll_name_, hit_pass_name_)) {
    ldmx_log(fatal) << "Couldn't find input collection " << hit_coll_name_
                    << " with pass name " << hit_pass_name_;
    return;
  }
  const auto ecal_rec_hits =
      event.getCollection<ldmx::EcalHit>(hit_coll_name_, hit_pass_name_);

  float e_total = 0;
  for (const auto& h : ecal_rec_hits) e_total += h.getEnergy();

  std::vector<ldmx::CaloCluster> pf_clusters;
  if (!single_cluster_) {
    DBScanClusterBuilder cb(min_hit_energy_, cluster_hit_dist_, cluster_z_bias_,
                            min_cluster_hit_mult_);
    std::vector<const ldmx::CalorimeterHit*> ptrs;
    for (const auto& h : ecal_rec_hits) ptrs.push_back(&h);
    std::vector<std::vector<const ldmx::CalorimeterHit*> > all_hit_ptrs =
        cb.runDBSCAN(ptrs);

    for (const auto& hit_ptrs : all_hit_ptrs) {
      ldmx::CaloCluster cl;
      cb.fillClusterInfoFromHits(&cl, hit_ptrs, log_energy_weight_);
      pf_clusters.push_back(cl);
    }
  } else {  // create a single, large cluster

    ldmx::CaloCluster cl;
    std::vector<const ldmx::CalorimeterHit*> ptrs;
    ptrs.reserve(ecal_rec_hits.size());
    for (const auto& h : ecal_rec_hits) {
      ptrs.push_back(&h);
    }
    DBScanClusterBuilder dummy;
    dummy.fillClusterInfoFromHits(&cl, ptrs, log_energy_weight_);
    pf_clusters.push_back(cl);
  }

  std::sort(pf_clusters.begin(), pf_clusters.end(),
            [](ldmx::CaloCluster a, ldmx::CaloCluster b) {
              return a.getEnergy() > b.getEnergy();
            });
  event.add(cluster_coll_name_, pf_clusters);
  event.add("EcalTotalEnergy" + suffix_, e_total);
}

}  // namespace recon

DECLARE_PRODUCER(recon::PFEcalClusterProducer);
