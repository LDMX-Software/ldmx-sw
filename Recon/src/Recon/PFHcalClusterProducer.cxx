#include "Recon/PFHcalClusterProducer.h"

#include "Hcal/Event/HcalCluster.h"
#include "Hcal/Event/HcalHit.h"
#include "Recon/DBScanClusterBuilder.h"
#include "Recon/Event/CaloCluster.h"
#include "Recon/Event/CalorimeterHit.h"
#include "TFitResult.h"
#include "TGraph.h"

namespace recon {

void PFHcalClusterProducer::configure(framework::config::Parameters& ps) {
  hit_coll_name_ = ps.get<std::string>("hit_coll_name");
  hit_pass_name_ = ps.get<std::string>("hit_pass_name");
  cluster_coll_name_ = ps.get<std::string>("cluster_coll_name");
  suffix_ = ps.get<std::string>("suffix", "");
  single_cluster_ = ps.get<bool>("do_single_cluster");
  log_energy_weight_ = ps.get<bool>("log_energy_weight");
  // DBScan parameters
  min_cluster_hit_mult_ = ps.get<int>("min_cluster_hit_mult");
  cluster_hit_dist_ = ps.get<double>("cluster_hit_dist");
  cluster_z_bias_ = ps.get<double>("clusterZBias", 1);
  min_hit_energy_ = ps.get<double>("min_hit_energy");
}

void PFHcalClusterProducer::produce(framework::Event& event) {
  if (!event.exists(hit_coll_name_, hit_pass_name_)) {
    ldmx_log(fatal) << "Couldn't find input collection " << hit_coll_name_
                    << "_" << hit_pass_name_;
    return;
  }
  const auto hcal_rec_hits =
      event.getCollection<ldmx::HcalHit>(hit_coll_name_, hit_pass_name_);
  float e_total = 0;
  for (const auto& h : hcal_rec_hits) e_total += h.getEnergy();

  std::vector<ldmx::CaloCluster> pf_clusters;
  if (!single_cluster_) {
    // construct DBScan
    DBScanClusterBuilder cb(min_hit_energy_, cluster_hit_dist_, cluster_z_bias_,
                            min_cluster_hit_mult_);
    std::vector<const ldmx::CalorimeterHit*> ptrs;
    for (const auto& h : hcal_rec_hits) ptrs.push_back(&h);
    std::vector<std::vector<const ldmx::CalorimeterHit*> > all_hit_ptrs =
        cb.runDBSCAN(ptrs);

    for (const auto& hit_ptrs : all_hit_ptrs) {
      ldmx::CaloCluster cl;
      cb.fillClusterInfoFromHits(&cl, hit_ptrs, log_energy_weight_);
      pf_clusters.push_back(cl);
    }

  } else {
    ldmx::CaloCluster cl;
    std::vector<const ldmx::CalorimeterHit*> ptrs;
    ptrs.reserve(hcal_rec_hits.size());
    for (const auto& h : hcal_rec_hits) {
      ptrs.push_back(&h);
    }
    DBScanClusterBuilder dummy;
    dummy.fillClusterInfoFromHits(&cl, ptrs, log_energy_weight_);
    pf_clusters.push_back(cl);
  }

  // sort
  std::sort(pf_clusters.begin(), pf_clusters.end(),
            [](ldmx::CaloCluster a, ldmx::CaloCluster b) {
              return a.getEnergy() > b.getEnergy();
            });
  event.add(cluster_coll_name_, pf_clusters);
  event.add("HcalTotalEnergy" + suffix_, e_total);
}

}  // namespace recon

DECLARE_PRODUCER(recon::PFHcalClusterProducer);
