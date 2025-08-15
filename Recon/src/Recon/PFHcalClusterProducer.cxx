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
  hitCollName_ = ps.get<std::string>("hitCollName");
  hitPassName_ = ps.get<std::string>("hitPassName");
  clusterCollName_ = ps.get<std::string>("clusterCollName");
  suffix_ = ps.get<std::string>("suffix", "");
  singleCluster_ = ps.get<bool>("doSingleCluster");
  logEnergyWeight_ = ps.get<bool>("logEnergyWeight");
  // DBScan parameters
  minClusterHitMult_ = ps.get<int>("minClusterHitMult");
  clusterHitDist_ = ps.get<double>("clusterHitDist");
  clusterZBias_ = ps.get<double>("clusterZBias", 1);
  min_hit_energy_ = ps.get<double>("minHitEnergy");
}

void PFHcalClusterProducer::produce(framework::Event& event) {
  if (!event.exists(hitCollName_, hitPassName_)) {
    ldmx_log(fatal) << "Couldn't find input collection " << hitCollName_ << "_"
                    << hitPassName_;
    return;
  }
  const auto hcalRecHits =
      event.getCollection<ldmx::HcalHit>(hitCollName_, hitPassName_);
  float eTotal = 0;
  for (const auto& h : hcalRecHits) eTotal += h.getEnergy();

  std::vector<ldmx::CaloCluster> pfClusters;
  if (!singleCluster_) {
    // construct DBScan
    DBScanClusterBuilder cb(min_hit_energy_, clusterHitDist_, clusterZBias_,
                            minClusterHitMult_);
    std::vector<const ldmx::CalorimeterHit*> ptrs;
    for (const auto& h : hcalRecHits) ptrs.push_back(&h);
    std::vector<std::vector<const ldmx::CalorimeterHit*> > all_hit_ptrs =
        cb.runDBSCAN(ptrs);

    for (const auto& hit_ptrs : all_hit_ptrs) {
      ldmx::CaloCluster cl;
      cb.fillClusterInfoFromHits(&cl, hit_ptrs, logEnergyWeight_);
      pfClusters.push_back(cl);
    }

  } else {
    ldmx::CaloCluster cl;
    std::vector<const ldmx::CalorimeterHit*> ptrs;
    ptrs.reserve(hcalRecHits.size());
    for (const auto& h : hcalRecHits) {
      ptrs.push_back(&h);
    }
    DBScanClusterBuilder dummy;
    dummy.fillClusterInfoFromHits(&cl, ptrs, logEnergyWeight_);
    pfClusters.push_back(cl);
  }

  // sort
  std::sort(pfClusters.begin(), pfClusters.end(),
            [](ldmx::CaloCluster a, ldmx::CaloCluster b) {
              return a.getEnergy() > b.getEnergy();
            });
  event.add(clusterCollName_, pfClusters);
  event.add("HcalTotalEnergy" + suffix_, eTotal);
}

}  // namespace recon

DECLARE_PRODUCER(recon::PFHcalClusterProducer);
