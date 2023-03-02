#include "Trigger/TrigElectronProducer.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "SimCore/Event/SimParticle.h"
#include "Trigger/Event/TrigCaloCluster.h"
#include "DetDescr/EcalGeometry.h"

namespace trigger {

void TrigElectronProducer::configure(framework::config::Parameters& ps) {
  spCollName_ = ps.getParameter<std::string>("scoringPlaneCollName");
  clusterCollName_ = ps.getParameter<std::string>("clusterCollName");
  eleCollName_ = ps.getParameter<std::string>("eleCollName");
}

void TrigElectronProducer::produce(framework::Event& event) {

  if (!event.exists(clusterCollName_)) return;
  auto ecalClusters{
      event.getObject<TrigCaloClusterCollection>(clusterCollName_)};

  if (!event.exists(spCollName_)) return;
  const std::vector<ldmx::SimTrackerHit> TargetSPHit = event.getCollection<ldmx::SimTrackerHit>(spCollName_);
  // ldmx::SimTrackerHit targetPrimary;
  // std::map<int,int> tk_to_iTargetSPHit;
  float xT=0, yT=0;
  for(const auto& hit : TargetSPHit){
    if(hit.getTrackID()!=1) continue;
    if( !(abs(hit.getPdgID())==11) ) continue;
    auto xyz = hit.getPosition();
    if( xyz[2]<0 || xyz[2]>1 ) continue; // select one sp
    xT=xyz[0];
    yT=xyz[1];
    // more details:
    // https://github.com/LDMX-Software/ldmx-analysis/blob/ch/dev/src/TriggerAnalyzer.cxx
  }

  std::vector<ldmx::SimParticle> eles;
  for(const auto &clus : ecalClusters){
    ldmx::SimParticle el;

    const float dX = clus.x() - xT;
    float R = clus.e()*(11500/4000.); // convert 11.5m/4GeV (in mm/MeV)
    float zd = 240.; // 240mm z detector
    float a = dX/zd;
    float b = (dX*dX+zd*zd)/(2*R*zd);

    float pred_px = clus.e() * (-b+a*sqrt(1+a*a-b*b))/(1+a*a);
    float pred_py = (clus.y() - yT) * 16; //mev/mm
    float pred_pz = sqrt(std::max(pow(clus.e(),2) - (pow(pred_px,2)+pow(pred_py,2)),0.));

    // produce el
    el.setEnergy(clus.e());
    el.setPdgID(11);
    el.setCharge(-1);
    el.setMass(0.000511);
    el.setVertex(0, xT, yT);
    el.setEndPoint(clus.x(), clus.y(), clus.z());
    el.setMomentum(pred_px, pred_py, pred_pz);
    eles.push_back(el);
  }
  event.add(eleCollName_, eles);

}

void TrigElectronProducer::onFileOpen() {
  ldmx_log(debug) << "Opening file!";

  return;
}

void TrigElectronProducer::onFileClose() {
  ldmx_log(debug) << "Closing file!";

  return;
}

void TrigElectronProducer::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  return;
}

void TrigElectronProducer::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}

}  // namespace trigger

DECLARE_PRODUCER_NS(trigger, TrigElectronProducer);
