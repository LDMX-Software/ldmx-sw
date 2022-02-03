#include "Trigger/TrigEcalClusterProducer.h"
#include "Trigger/IdealClusterBuilder.h"

#include "Trigger/Event/TrigCaloHit.h"
#include "Trigger/Event/TrigCaloCluster.h"

#include "DetDescr/EcalHexReadout.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Recon/Event/HgcrocTrigDigi.h"

namespace trigger {

void TrigEcalClusterProducer::configure(framework::config::Parameters& ps) {
  hitCollName_ = ps.getParameter<std::string>("hitCollName");
  clusterCollName_ = ps.getParameter<std::string>("clusterCollName");
 
}

void TrigEcalClusterProducer::produce(framework::Event& event) {
  const ecal::EcalTriggerGeometry& geom =
      getCondition<ecal::EcalTriggerGeometry>(
          ecal::EcalTriggerGeometry::CONDITIONS_OBJECT_NAME);
  const ldmx::EcalHexReadout& hexReadout = getCondition<ldmx::EcalHexReadout>(
      ldmx::EcalHexReadout::CONDITIONS_OBJECT_NAME);

  if (!event.exists(hitCollName_)) return;
  auto ecalTrigDigis{
      event.getObject<ldmx::HgcrocTrigDigiCollection>(hitCollName_)};

  std::vector<Hit> hits{};
  for (const auto& trigDigi : ecalTrigDigis) {
    ldmx::EcalTriggerID tid(trigDigi.getId());
    float sie = hgc_compression_factor_ * trigDigi.linearPrimitive() *
                gain_ * mVtoMeV_;  // in MeV, before layer corrections
    float e = (sie / mipSiEnergy_ * layerWeights.at(tid.layer()) + sie) *
              secondOrderEnergyCorrection_;
    
    double x, y, z;
    const auto center_ecalID = geom.centerInTriggerCell(tid);
    hexReadout.getCellAbsolutePosition(center_ecalID,x,y,z);
    std::tie(x,y) = geom.globalPosition( tid );

    // produce Hit object for clustering class
    Hit hit;
    hit.e = e;
    hit.x = x;
    hit.y = y;
    hit.z = z;
    hit.layer = tid.layer();
    hit.cell_id = tid.getTriggerCellID();
    hit.module_id = tid.module();
    hit.idx = hits.size();
    hits.push_back(hit);
  }

  // move to once per run
  ClusterGeometry myGeo;
  if(!myGeo.is_initialized){ 
    for(int imod=0; imod<7; imod++){
      for(int icell=0; icell<48; icell++){
        ldmx::EcalTriggerID id(0, imod, icell);
        auto xy = geom.globalPosition( id );
        myGeo.AddTP(id.raw(), icell, imod, xy.first, xy.second);
      }
    }
    myGeo.Initialize();
  }
  IdealClusterBuilder builder;
  builder.SetClusterGeo( &myGeo );
  for(const auto& h : hits) builder.AddHit(h);
  // TODO: add options to configure the builder here
  builder.BuildClusters();
  auto clusters = builder.GetClusters();

  TrigCaloClusterCollection trigClusters;
  for (const auto &c : clusters) {
    TrigCaloCluster t(c.x, c.y, c.z, c.e);
    t.setXYZerr(c.xx, c.yy, c.zz);
    t.setdxdz(c.dxdz);
    t.setdydz(c.dydz);
    t.setdxdze(c.dxdze);
    t.setdydze(c.dydze);
    t.set3D(!c.is2D);
    t.setLayer(c.layer);
    t.setFirstLayer(c.first_layer);
    t.setLastLayer(c.last_layer);
    t.setDepth(c.depth);    
    int nTP=0;
    if(c.is2D){
      nTP = c.hits.size();
    } else {
      for (const auto & c2d : c.clusters2d)
        nTP += c2d.hits.size();
    }
    t.setNTP(nTP);
    trigClusters.push_back(t);
  }

  event.add(clusterCollName_, trigClusters);
}

void TrigEcalClusterProducer::onFileOpen() {
  ldmx_log(debug) << "Opening file!";

  return;
}

void TrigEcalClusterProducer::onFileClose() {
  ldmx_log(debug) << "Closing file!";

  return;
}

void TrigEcalClusterProducer::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  return;
}

void TrigEcalClusterProducer::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}

}  // namespace trigger

DECLARE_PRODUCER_NS(trigger, TrigEcalClusterProducer);
