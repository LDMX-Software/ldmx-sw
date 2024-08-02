
#include "DQM/PFDQM.h"
namespace dqm {

PFDQM::PFDQM(const std::string &name, framework::Process &process)
    : framework::Analyzer(name, process) {}

void PFDQM::configure(framework::config::Parameters &ps) {
  hcal_rec_coll_name_ = ps.getParameter<std::string>("hcal_rec_coll_name");
  hcal_rec_pass_name_ = ps.getParameter<std::string>("hcal_rec_pass_name");
  ecal_rec_coll_name_ = ps.getParameter<std::string>("ecal_rec_coll_name");
  ecal_rec_pass_name_ = ps.getParameter<std::string>("ecal_rec_pass_name");
  hcal_sim_coll_name_ = ps.getParameter<std::string>("hcal_sim_coll_name");
  hcal_sim_pass_name_ = ps.getParameter<std::string>("hcal_sim_pass_name");
  ecal_sim_coll_name_ = ps.getParameter<std::string>("ecal_sim_coll_name");
  ecal_sim_pass_name_ = ps.getParameter<std::string>("ecal_sim_pass_name");
}

void PFDQM::analyze(const framework::Event &event) {
  if (!event.exists("EcalScoringPlaneHits")) return;
  const auto ecalSpHits = event.getCollection<ldmx::SimTrackerHit>("EcalScoringPlaneHits");
  // if (!event.exists("SimParticles")) return;
  // const auto particle_map = event.getMap<int,ldmx::SimParticle>("SimParticles");

  float truthHCal_x{0}, truthHCal_y{0};
  for(const auto &spHit : ecalSpHits){
    // if ( simIDs.count(spHit.getTrackID()) && fabs(240-spHit.getPosition()[2])<0.1  && spHit.getMomentum()[2] > 0 ){ 
    if ( spHit.getTrackID()==1 && fabs(840-spHit.getPosition()[2])<0.1  && spHit.getMomentum()[2] > 0 ){ 
      truthHCal_x = spHit.getPosition()[0];
      truthHCal_y = spHit.getPosition()[1];
    }
  }

  
  // Get the collection of PFDQM digitized hits if the exists
  const auto &hcalHits{
      event.getCollection<ldmx::HcalHit>(hcal_rec_coll_name_, hcal_rec_pass_name_)};
  const auto &hcalSimHits{
      event.getCollection<ldmx::SimCalorimeterHit>(hcal_sim_coll_name_, hcal_sim_pass_name_)};

  const auto &hcal_geometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);


  float hcal_totalE{0};
  float hcal_x{0},  hcal_xe{0},  hcal_xle{0};
  float hcal_y{0},  hcal_ye{0},  hcal_yle{0};
  float hcal_x2{0},  hcal_x2e{0},  hcal_x2le{0};
  float hcal_y2{0},  hcal_y2e{0},  hcal_y2le{0};
  float hcal_z{0},  hcal_ze{0},  hcal_zle{0};
  float hcal_t{0},  hcal_te{0},  hcal_tle{0};
  float hcal_xsum{0},  hcal_xsume{0},  hcal_xsumle{0};  
  float hcal_ysum{0},  hcal_ysume{0},  hcal_ysumle{0};
  float hcal_zsum{0},  hcal_zsume{0},  hcal_zsumle{0};
  float hcal_minE{0.05};
  
  for (const ldmx::HcalHit &hit : hcalHits) {
    ldmx::HcalID id(hit.getID());
    const auto orientation{hcal_geometry.getScintillatorOrientation(id)};
    const auto section{id.section()};
    const auto layer{id.layer()};
    const auto strip{id.strip()};

    const auto t{hit.getTime()};
    const auto e{hit.getEnergy()};
    const auto x{hit.getXPos()};
    const auto y{hit.getYPos()};
    const auto z{hit.getZPos()};
    switch (orientation) {
      case ldmx::HcalGeometry::ScintillatorOrientation::horizontal:
        histograms_.fill("hcal_x", x);
	hcal_x += x;
	hcal_xe += x*e;
	hcal_xle += x*log(e/hcal_minE);
	hcal_y2 += y;
	hcal_y2e += y*e;
	hcal_y2le += y*log(e/hcal_minE);
	hcal_xsum += 1;
	hcal_xsume += e;
	hcal_xsumle += log(e/hcal_minE);
        break;
      case ldmx::HcalGeometry::ScintillatorOrientation::vertical:
        histograms_.fill("hcal_y", y);
	hcal_y += y;
	hcal_ye += y*e;
	hcal_yle += y*log(e/hcal_minE);
	hcal_x2 += x;
	hcal_x2e += x*e;
	hcal_x2le += x*log(e/hcal_minE);
	hcal_ysum += 1;
	hcal_ysume += e;
	hcal_ysumle += log(e/hcal_minE);
        break;
      case ldmx::HcalGeometry::ScintillatorOrientation::depth:
        histograms_.fill("hcal_z", z);
	hcal_z += z;
	hcal_ze += z*e;
	hcal_zle += z*log(e/hcal_minE);
	hcal_zsum += 1;
	hcal_zsume += e;
	hcal_zsumle += log(e/hcal_minE);
        break;
    }
    hcal_totalE += e;
    hcal_t += t;
    hcal_te += t*e;
    hcal_tle += t*log(e/hcal_minE);
    
    histograms_.fill("hcal_hit_time", t);
    histograms_.fill("hcal_layer", layer);
  }
  histograms_.fill("hcal_total_energy", hcal_totalE);
  histograms_.fill("hcal_dx_truthHcal",   hcal_x/hcal_xsum - truthHCal_x);
  histograms_.fill("hcal_dx_truthHcal_e", hcal_xe/hcal_xsume - truthHCal_x);
  histograms_.fill("hcal_dx_truthHcal_le",hcal_xle/hcal_xsumle - truthHCal_x);
  histograms_.fill("hcal_dy_truthHcal",   hcal_y/hcal_ysum - truthHCal_y);
  histograms_.fill("hcal_dy_truthHcal_e", hcal_ye/hcal_ysume - truthHCal_y);
  histograms_.fill("hcal_dy_truthHcal_le",hcal_yle/hcal_ysumle - truthHCal_y);
  histograms_.fill("hcal_dz_truthHcal",   hcal_z/hcal_zsum);
  histograms_.fill("hcal_dz_truthHcal_e", hcal_ze/hcal_zsume);
  histograms_.fill("hcal_dz_truthHcal_le",hcal_zle/hcal_zsumle);
  histograms_.fill("hcal_dx2_truthHcal",   hcal_x2/hcal_ysum - truthHCal_x);
  histograms_.fill("hcal_dx2_truthHcal_e", hcal_x2e/hcal_ysume - truthHCal_x);
  histograms_.fill("hcal_dx2_truthHcal_le",hcal_x2le/hcal_ysumle - truthHCal_x);
  histograms_.fill("hcal_dy2_truthHcal",   hcal_y2/hcal_xsum - truthHCal_y);
  histograms_.fill("hcal_dy2_truthHcal_e", hcal_y2e/hcal_xsume - truthHCal_y);
  histograms_.fill("hcal_dy2_truthHcal_le",hcal_y2le/hcal_xsumle - truthHCal_y);
  histograms_.fill("x_truthHcal", truthHCal_x);
  histograms_.fill("y_truthHcal", truthHCal_y);
  histograms_.fill("hcal_dx_vs_truthHcal", truthHCal_x, hcal_x/hcal_xsum);
  histograms_.fill("hcal_dy_vs_truthHcal", truthHCal_y, hcal_y/hcal_ysum);
  //histograms_.fill("hcal_dz_vs_truthHcal", truthHCal_z, hcal_z/hcal_zsum);
  histograms_.fill("hcal_dx2_vs_truthHcal", truthHCal_x, hcal_x2/hcal_ysum);
  histograms_.fill("hcal_dy2_vs_truthHcal", truthHCal_y, hcal_y2/hcal_xsum);

  histograms_.fill("hcal_t",   hcal_t/(hcal_xsum+hcal_ysum+hcal_zsum));
  histograms_.fill("hcal_t_e", hcal_te/(hcal_xsume+hcal_ysume+hcal_zsume));
  histograms_.fill("hcal_t_le",hcal_tle/(hcal_xsumle+hcal_ysumle+hcal_zsumle));

  float hcal_totalSimE{0};
  for (const auto &hit : hcalSimHits) {
    hcal_totalSimE += hit.getEdep();
  }
  histograms_.fill("hcal_total_sim_energy", hcal_totalSimE);
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, PFDQM)
