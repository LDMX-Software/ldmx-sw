#include "Recon/PFEcalClusterProducer.h"
#include "Recon/DBScanClusterBuilder.h"

#include "Recon/Event/CalorimeterHit.h"
#include "Recon/Event/CaloCluster.h"

namespace recon {

void PFEcalClusterProducer::configure(framework::config::Parameters& ps) {
  hitCollName_ = ps.getParameter<std::string>("hitCollName");
  clusterCollName_ = ps.getParameter<std::string>("clusterCollName"); 
  suffix_ = ps.getParameter<std::string>("suffix","");
  singleCluster_ = ps.getParameter<bool>("doSingleCluster");
  logEnergyWeight_ = ps.getParameter<bool>("logEnergyWeight");
  //DBScan parameters
  minClusterHitMult_ = ps.getParameter<int>("minClusterHitMult");
  clusterHitDist_ = ps.getParameter<double>("clusterHitDist");
  clusterZBias_ = ps.getParameter<double>("clusterZBias",1);
  minHitEnergy_ = ps.getParameter<double>("minHitEnergy");
}

void PFEcalClusterProducer::produce(framework::Event& event) {

  if (!event.exists(hitCollName_)) return;
  const auto ecalRecHits = event.getCollection<ldmx::EcalHit>(hitCollName_);
  const auto& geo = getCondition<ldmx::EcalGeometry>(ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);

  float eTotal=0;
  for (const auto &h : ecalRecHits) eTotal += h.getEnergy();

  std::vector<ldmx::CaloCluster> pfClusters;
  if(!singleCluster_){ 

    DBScanClusterBuilder cb(minHitEnergy_, clusterHitDist_, clusterZBias_, minClusterHitMult_);
    std::vector<const ldmx::CalorimeterHit*> ptrs;
    for(const auto & h : ecalRecHits) ptrs.push_back(&h);
    std::vector<std::vector<const ldmx::CalorimeterHit*> > all_hit_ptrs = cb.runDBSCAN(ptrs,false);

    for(const auto hit_ptrs : all_hit_ptrs){
      ldmx::CaloCluster cl;
      cb.fillClusterInfoFromHits(&cl, hit_ptrs, logEnergyWeight_);
      pfClusters.push_back(cl);
    }    
  } else { // create a single, large cluster

    ldmx::CaloCluster cl;
    std::vector<const ldmx::CalorimeterHit*> ptrs; 
    ptrs.reserve(ecalRecHits.size());
    for (const auto &h : ecalRecHits) {
      ptrs.push_back(&h);
    }
    DBScanClusterBuilder dummy;
    dummy.fillClusterInfoFromHits(&cl, ptrs, logEnergyWeight_);
    pfClusters.push_back(cl);
  }

  std::sort(pfClusters.begin(), pfClusters.end(),
	    [](ldmx::CaloCluster a, ldmx::CaloCluster b) {
	      return a.getEnergy() > b.getEnergy();
	    });
  event.add(clusterCollName_, pfClusters);
  event.add("EcalTotalEnergy"+suffix_, eTotal);
}

void PFEcalClusterProducer::onFileOpen() {
  ldmx_log(debug) << "Opening file!";

  return;
}

void PFEcalClusterProducer::onFileClose() {
  ldmx_log(debug) << "Closing file!";

  return;
}

void PFEcalClusterProducer::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  return;
}

void PFEcalClusterProducer::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}

}  // namespace recon

DECLARE_PRODUCER_NS(recon, PFEcalClusterProducer);
