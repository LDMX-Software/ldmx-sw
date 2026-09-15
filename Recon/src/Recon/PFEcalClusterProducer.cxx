#include "Recon/PFEcalClusterProducer.h"

#include "Recon/DBScanClusterBuilder.h"
#include "Recon/Event/CaloCluster.h"
#include "Recon/Event/CalorimeterHit.h"

namespace recon {

void PFEcalClusterProducer::configure(framework::config::Parameters& ps) {
  hit_coll_name_ = ps.get<std::string>("hit_coll_name");
  hit_pass_name_ = ps.get<std::string>("hit_pass_name");
  cluster_coll_name_ = ps.get<std::string>("cluster_coll_name");
  suffix_ = ps.get<std::string>("suffix", "");
  single_cluster_ = ps.get<bool>("do_single_cluster");
  log_energy_weight_ = ps.get<bool>("log_energy_weight");
  // DBScan parameters
  min_cluster_hit_mult_ = ps.get<int>("min_cluster_hit_mult");
  cluster_hit_dist_ = ps.get<double>("cluster_hit_dist");
  cluster_z_bias_ = ps.get<double>("cluster_z_bias", 1);
  min_hit_energy_ = ps.get<double>("min_hit_energy");
  save_hit_contribs_ = ps.get<bool>("save_hit_contribs", true);
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
      cb.fillClusterInfoFromHits(&cl, hit_ptrs, log_energy_weight_,
                                 save_hit_contribs_);
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
    dummy.fillClusterInfoFromHits(&cl, ptrs, log_energy_weight_,
                                  save_hit_contribs_);
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
