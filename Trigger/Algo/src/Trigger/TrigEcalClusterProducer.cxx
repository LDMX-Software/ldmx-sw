#include "Trigger/TrigEcalClusterProducer.h"

#include "DetDescr/EcalGeometry.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Recon/Event/HgcrocTrigDigi.h"
#include "Trigger/Event/TrigCaloCluster.h"
#include "Trigger/Event/TrigCaloHit.h"
#include "Trigger/IdealClusterBuilder.h"

namespace trigger {

void TrigEcalClusterProducer::configure(framework::config::Parameters& ps) {
  hit_coll_name_ = ps.get<std::string>("hit_coll_name");
  cluster_coll_name_ = ps.get<std::string>("cluster_coll_name");
  hit_coll_passname_ = ps.get<std::string>("hit_coll_passname");
  hit_coll_name_events_passname_ =
      ps.get<std::string>("hit_coll_name_events_passname");
}

void TrigEcalClusterProducer::produce(framework::Event& event) {
  const ecal::EcalTriggerGeometry& geom =
      getCondition<ecal::EcalTriggerGeometry>(
          ecal::EcalTriggerGeometry::CONDITIONS_OBJECT_NAME);

  if (!event.exists(hit_coll_name_, hit_coll_name_events_passname_)) return;
  auto ecal_trig_digis{event.getObject<ldmx::HgcrocTrigDigiCollection>(
      hit_coll_name_, hit_coll_passname_)};

  std::vector<Hit> hits{};
  EcalTpToE cvt;
  for (const auto& trig_digi : ecal_trig_digis) {
    ldmx::EcalTriggerID tid(trig_digi.getId());
    float e = cvt.calc(trig_digi.linearPrimitive(), tid.layer());

    // float sie = hgc_compression_factor_ * trigDigi.linearPrimitive() *
    //             gain_ * m_vto_me_v_;  // in MeV, before layer corrections
    // float e = (sie / mip_si_energy_ * layerWeights.at(tid.layer()) + sie) *
    //           second_order_energy_correction_;

    double x, y, z;
    // const auto center_ecalID = geom.centerInTriggerCell(tid);
    // const ldmx::EcalGeometry& hexReadout = getCondition<ldmx::EcalGeometry>(
    // ldmx::EcalGeometry::CONDITIONS_OBJECT_NAME);
    // hexReadout.getCellAbsolutePosition(center_ecalID,x,y,z);
    // std::tie(x,y) = geom.globalPosition( tid );
    std::tie(x, y, z) = geom.globalPosition(tid);

    // produce Hit object for clustering class
    Hit hit;
    hit.e_ = e;
    hit.x_ = x;
    hit.y_ = y;
    hit.z_ = z;
    hit.layer_ = tid.layer();
    hit.cell_id_ = tid.getTriggerCellID();
    hit.module_id_ = tid.module();
    hit.idx_ = hits.size();
    hits.push_back(hit);
  }

  // move to once per run
  ClusterGeometry my_geo;
  if (!my_geo.is_initialized_) {
    for (int imod = 0; imod < 7; imod++) {
      for (int icell = 0; icell < 48; icell++) {
        ldmx::EcalTriggerID id(0, imod, icell);
        auto [xx, yy, zz] = geom.globalPosition(id);
        my_geo.addTp(id.raw(), icell, imod, xx, yy);
      }
    }
    my_geo.initialize();
  }
  IdealClusterBuilder builder;
  builder.setClusterGeo(&my_geo);
  for (const auto& h : hits) builder.addHit(h);
  // TODO: add options to configure the builder here
  builder.buildClusters();
  auto clusters = builder.getClusters();

  TrigCaloClusterCollection trig_clusters;
  for (const auto& c : clusters) {
    TrigCaloCluster t(c.x_, c.y_, c.z_, c.e_);
    t.setXYZerr(c.xx_, c.yy_, c.zz_);
    t.setdxdz(c.dxdz_);
    t.setdydz(c.dydz_);
    t.setdxdze(c.dxdze_);
    t.setdydze(c.dydze_);
    t.set3D(!c.is_2d_);
    t.setLayer(c.layer_);
    t.setFirstLayer(c.first_layer_);
    t.setLastLayer(c.last_layer_);
    t.setDepth(c.depth_);
    int n_tp = 0;
    if (c.is_2d_) {
      n_tp = c.hits_.size();
    } else {
      for (const auto& c2d : c.clusters2d_) n_tp += c2d.hits_.size();
    }
    t.setNTP(n_tp);
    trig_clusters.push_back(t);
  }

  event.add(cluster_coll_name_, trig_clusters);
}
}  // namespace trigger

DECLARE_PRODUCER(trigger::TrigEcalClusterProducer);
