//#include "DetDescr/EcalGeometry.h"
// #include "Recon/Event/HgcrocDigiCollection.h"

#include "Recon/PFEcalClusterProducer.h"

#include "Ecal/Event/EcalHit.h"
#include "Ecal/Event/EcalCluster.h"

namespace recon {

void PFEcalClusterProducer::configure(framework::config::Parameters& ps) {
  hitCollName_ = ps.getParameter<std::string>("hitCollName");
  clusterCollName_ = ps.getParameter<std::string>("clusterCollName"); 
}

void PFEcalClusterProducer::produce(framework::Event& event) {

  if (!event.exists(hitCollName_)) return;
  const auto ecalRecHits = event.getCollection<ldmx::EcalHit>(hitCollName_);
  std::cout << "found " << ecalRecHits.size() << " hits" << std::endl;

  std::vector<ldmx::EcalCluster> pfClusters;
  if(trivialCluster_){ 
    ldmx::EcalCluster cl;
    float e(0),x(0),y(0),z(0),xx(0),yy(0),zz(0),n(0);
    // can configure later
    float w = 1; // weight
    float sumw = 0;
    float minEnergy=0; // min hit energy to consider
    for(const auto &h : ecalRecHits){
      if (h.getEnergy() < minEnergy) continue;
      // w = log( h.getEnergy() - log(minEnergy) );
      e += h.getEnergy();
      x += w * h.getXPos();
      y += w * h.getYPos();
      z += w * h.getZPos();
      xx += w * h.getXPos() * h.getXPos();
      yy += w * h.getYPos() * h.getYPos();
      zz += w * h.getZPos() * h.getZPos();
      n += 1;
      sumw += w;
    }
    x /= sumw; // now is <x>
    y /= sumw;
    z /= sumw;
    xx /= sumw; // now is <x^2>
    yy /= sumw;
    zz /= sumw;
    xx = sqrt(xx - x * x); //now is sqrt(<x^2>-<x>^2)
    yy = sqrt(yy - y * y); 
    zz = sqrt(zz - z * z);
    cl.setEnergy(e);
    cl.setNHits(n);
    cl.setCentroidXYZ(x,y,z);
    // TODO add RMS
    pfClusters.push_back(cl);
  }
  event.add(clusterCollName_, pfClusters);
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
