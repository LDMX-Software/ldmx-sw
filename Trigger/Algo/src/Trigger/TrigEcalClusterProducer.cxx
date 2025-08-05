#include "Trigger/TrigEcalClusterProducer.h"

#include "DetDescr/EcalGeometry.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Recon/Event/HgcrocTrigDigi.h"
#include "Trigger/Event/TrigCaloCluster.h"
#include "Trigger/Event/TrigCaloHit.h"
#include "Trigger/IdealClusterBuilder.h"

namespace trigger {

void TrigEcalClusterProducer::configure(framework::config::Parameters& ps) {
  hitCollName_ = ps.getParameter<std::string>("hitCollName");
  clusterCollName_ = ps.getParameter<std::string>("clusterCollName");
  hit_coll_passname_ = ps.getParameter<std::string>("hit_coll_passname");
  hit_coll_name_events_passname_ =
      ps.getParameter<std::string>("hit_coll_name_events_passname");
}

void TrigEcalClusterProducer::produce(framework::Event& event) {
  const ecal::EcalTriggerGeometry& geom =
      getCondition<ecal::EcalTriggerGeometry>(
          ecal::EcalTriggerGeometry::CONDITIONS_OBJECT_NAME);

  if (!event.exists(hitCollName_, hit_coll_name_events_passname_)) return;
  auto ecalTrigDigis{event.getObject<ldmx::HgcrocTrigDigiCollection>(
      hitCollName_, hit_coll_passname_)};

  std::vector<Hit> hits_{};
  ecalTpToE cvt;
  for (const auto& trigDigi : ecalTrigDigis) {
    ldmx::EcalTriggerID tid(trigDigi.getId());
    float e = cvt.calc(trigDigi.linearPrimitive(), tid.layer());

    // float sie = hgc_compression_factor_ * trigDigi.linearPrimitive() *
    //             gain_ * mVtoMeV_;  // in MeV, before layer_ corrections
    // float e = (sie / mipSiEnergy_ * layerWeights.at(tid.layer()) + sie) *
    //           second_order_energy_correction_;

    double x_, y_, z_;
    // const auto center_ecalID = geom.centerInTriggerCell(tid);
    // const ldmx::EcalGeometry& hexReadout = getCondition<ldmx::EcalGeometry>(
    // ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);
    // hexReadout.getCellAbsolutePosition(center_ecalID,x_,y_,z_);
    // std::tie(x_,y_) = geom.globalPosition( tid );
    std::tie(x_, y_, z_) = geom.globalPosition(tid);

    // produce Hit object for clustering class
    Hit hit;
    hit.e = e;
    hit.x_ = x_;
    hit.y_ = y_;
    hit.z_ = z_;
    hit.layer_ = tid.layer();
    hit.cell_id = tid.getTriggerCellID();
    hit.module_id = tid.module();
    hit.idx = hits_.size();
    hits_.push_back(hit);
  }

  // move to once per run
  ClusterGeometry myGeo;
  if (!myGeo.is_initialized) {
    for (int imod = 0; imod < 7; imod++) {
      for (int icell = 0; icell < 48; icell++) {
        ldmx::EcalTriggerID id(0, imod, icell);
        auto [xx, yy, zz] = geom.globalPosition(id);
        myGeo.AddTP(id.raw(), icell, imod, xx, yy);
      }
    }
    myGeo.Initialize();
  }
  IdealClusterBuilder builder;
  builder.SetClusterGeo(&myGeo);
  for (const auto& h : hits_) builder.AddHit(h);
  // TODO: add options to configure the builder here
  builder.BuildClusters();
  auto clusters = builder.GetClusters();

  TrigCaloClusterCollection trigClusters;
  for (const auto& c : clusters) {
    TrigCaloCluster t(c.x_, c.y_, c.z_, c.e);
    t.setXYZerr(c.xx, c.yy, c.zz);
    t.setdxdz(c.dxdz);
    t.setdydz(c.dydz);
    t.setdxdze(c.dxdze);
    t.setdydze(c.dydze);
    t.set3D(!c.is2D);
    t.setLayer(c.layer_);
    t.setFirstLayer(c.first_layer);
    t.setLastLayer(c.last_layer);
    t.setDepth(c.depth);
    int nTP = 0;
    if (c.is2D) {
      nTP = c.hits_.size();
    } else {
      for (const auto& c2d : c.clusters2d) nTP += c2d.hits_.size();
    }
    t.setNTP(nTP);
    trigClusters.push_back(t);
  }

  event.add(clusterCollName_, trigClusters);
}
}  // namespace trigger

DECLARE_PRODUCER(trigger::TrigEcalClusterProducer);
