#include "Recon/PFHcalClusterProducer.h"

#include "Hcal/Event/HcalHit.h"
#include "Hcal/Event/HcalCluster.h"

namespace recon {

void PFHcalClusterProducer::configure(framework::config::Parameters& ps) {
  hitCollName_ = ps.getParameter<std::string>("hitCollName");
  clusterCollName_ = ps.getParameter<std::string>("clusterCollName"); 
}

void PFHcalClusterProducer::produce(framework::Event& event) {

  if (!event.exists(hitCollName_)) return;
  const auto hcalRecHits = event.getCollection<ldmx::HcalHit>(hitCollName_);
  std::cout << "found " << hcalRecHits.size() << " hits" << std::endl;

  std::vector<ldmx::HcalCluster> pfClusters;
  if(trivialCluster_){ 
    ldmx::HcalCluster cl;
    float e(0),x(0),y(0),z(0),xx(0),yy(0),zz(0),n(0);
    // can configure later
    float w = 1; // weight
    float sumw = 0;
    float minEnergy=0; // min hit energy to consider
    for(const auto &h : hcalRecHits){
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

void PFHcalClusterProducer::onFileOpen() {
  ldmx_log(debug) << "Opening file!";

  return;
}

void PFHcalClusterProducer::onFileClose() {
  ldmx_log(debug) << "Closing file!";

  return;
}

void PFHcalClusterProducer::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  return;
}

void PFHcalClusterProducer::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}

}  // namespace recon

DECLARE_PRODUCER_NS(recon, PFHcalClusterProducer);
