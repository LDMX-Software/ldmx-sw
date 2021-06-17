
#include "DQM/SimObjects.h"

#include "Framework/ProductTag.h"

#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "SimCore/Event/SimCalorimeterHit.h"

namespace dqm {

void SimObjects::configure(framework::config::Parameters &ps) {
  sim_pass_ = ps.getParameter<std::string>("sim_pass");
  return;
}

void SimObjects::onProcessStart() {
  getHistoDirectory();

  // create sim particles histograms 
  histograms_.create("SimParticles.E","Vertex Total Energy [MeV]",800,0.,8000.);
  histograms_.create("SimParticles.px","Vertex Momentum in x-Direction [MeV]",50,0.,500.);
  histograms_.create("SimParticles.py","Vertex Momentum in y-Direction [MeV]",50,0.,500.);
  histograms_.create("SimParticles.pz","Vertex Momentum in z-Direction [MeV]",400,0.,4000.);
  histograms_.create("SimParticles.time","Global Time of Creation [ns]",50,0.,10.);
  histograms_.create("SimParticles.pdg","PDG ID of Particle",201,-100,100);
  histograms_.create("SimParticles.x","Vertex x-Position [mm]",401,-200,200);
  histograms_.create("SimParticles.y","Vertex y-Position [mm]",401,-200,200);
  histograms_.create("SimParticles.z","Vertex z-Position [mm]",171,-700,1000.);
  histograms_.create("SimParticles.process","Creator Process Type",12,0,12);
  histograms_.create("SimParticles.track_id","Track ID of Particle",100,0,1000);
  histograms_.create("SimParticles.parent","Track ID of Parent",100,0,1000);
  histograms_.create("SimParticles.children","Track IDs of Children",100,0,1000);

  // create pn children histograms
  histograms_.create("pn_child.E","Vertex Total Energy [MeV]",800,0.,8000.);
  histograms_.create("pn_child.px","Vertex Momentum in x-Direction [MeV]",50,0.,500.);
  histograms_.create("pn_child.py","Vertex Momentum in y-Direction [MeV]",50,0.,500.);
  histograms_.create("pn_child.pz","Vertex Momentum in z-Direction [MeV]",400,0.,4000.);
  histograms_.create("pn_child.time","Global Time of Creation [ns]",50,0.,10.);
  histograms_.create("pn_child.pdg","PDG ID of Particle",201,-100,100);
  histograms_.create("pn_child.x","Vertex x-Position [mm]",401,-200,200);
  histograms_.create("pn_child.y","Vertex y-Position [mm]",401,-200,200);
  histograms_.create("pn_child.z","Vertex z-Position [mm]",171,-700,1000.);
  histograms_.create("pn_child.track_id","Track ID of Particle",100,0,1000);
  histograms_.create("pn_child.parent","Track ID of Parent",100,0,1000);
  histograms_.create("pn_child.children","Track IDs of Children",100,0,1000);
}

void SimObjects::createCalorimeterHists(const std::string& coll_name) {
  getHistoDirectory();
  histograms_.create(coll_name+".incidents","Incident Track IDs",100,0,1000);
  histograms_.create(coll_name+".tracks","Contributing Track IDs",100,0,1000);
  histograms_.create(coll_name+".pdg","Contributing PDG IDs",401,-200,200);
  histograms_.create(coll_name+".edep","Energy Deposited [MeV]",100,0,200);
  histograms_.create(coll_name+".x","Hit x-Position [mm]",401,-200,200);
  histograms_.create(coll_name+".y","Hit y-Position [mm]",401,-200,200);
  histograms_.create(coll_name+".z","Hit z-Position [mm]",171,-700,1000.);
  histograms_.create(coll_name+".time","Hit Time [ns]",50,0.,10.);
  histograms_.create(coll_name+".n_contribs","Number of Contributors",20,0.,20.);
  return;
}

void SimObjects::createTrackerHists(const std::string& coll_name) {
  getHistoDirectory();
  histograms_.create(coll_name+".particle_E","Particle E at Hit [MeV]",400,0,4000);
  histograms_.create(coll_name+".particle_px","Particle Momentum in x [MeV]",50,0.,500.);
  histograms_.create(coll_name+".particle_py","Particle Momentum in y [MeV]",50,0.,500.);
  histograms_.create(coll_name+".particle_pz","Particle Momentum in z [MeV]",400,0.,4000.);
  histograms_.create(coll_name+".edep","Energy Deposited [MeV]",100,0,200);
  histograms_.create(coll_name+".time","Hit Time [ns]",50,0.,10.);
  histograms_.create(coll_name+".x","Hit x-Position [mm]",401,-200,200);
  histograms_.create(coll_name+".y","Hit y-Position [mm]",401,-200,200);
  histograms_.create(coll_name+".z","Hit z-Position [mm]",171,-700,1000.);
  histograms_.create(coll_name+".track","Particle Track ID",100,0,1000);
  histograms_.create(coll_name+".pdg","Particle PDG ID",401,-200,200);
  return;
}

void SimObjects::analyze(const framework::Event &event) {
  static std::vector<framework::ProductTag> sp_maps, calo_colls, track_colls;
  if (sp_maps.empty()) {
    sp_maps = event.searchProducts("SimParticles","","");
    if (sp_maps.size() != 1) {
      ldmx_log(warn) << sp_maps.size() << " SimParticle maps which is not one!";
    }
  }
  if (calo_colls.empty()) {
    calo_colls = event.searchProducts("",sim_pass_,".*SimCalorimeterHit.*");
    for (auto pt : calo_colls)
      createCalorimeterHists(pt.name());
  }
  if (track_colls.empty()) {
    track_colls = event.searchProducts("",sim_pass_,".*SimTrackerHit.*");
    for (auto pt : track_colls)
      createTrackerHists(pt.name());
  }

  auto const& particle_map{event.getMap<int,ldmx::SimParticle>("SimParticles")};
  for (auto const& [track_id, particle] : particle_map) {
    auto const& momentum{particle.getMomentum()};
    auto const& vertex{particle.getVertex()};
    histograms_.fill("SimParticles.E", particle.getEnergy());
    histograms_.fill("SimParticles.px",momentum.at(0));
    histograms_.fill("SimParticles.py",momentum.at(1));
    histograms_.fill("SimParticles.pz",momentum.at(2));
    histograms_.fill("SimParticles.time",particle.getTime());
    histograms_.fill("SimParticles.pdg",particle.getPdgID());
    histograms_.fill("SimParticles.x", vertex.at(0));
    histograms_.fill("SimParticles.y", vertex.at(1));
    histograms_.fill("SimParticles.z", vertex.at(2));
    histograms_.fill("SimParticles.process", particle.getProcessType());
    histograms_.fill("SimParticles.track_id", track_id);
    for (auto const& parent : particle.getParents())
      histograms_.fill("SimParticles.parent", parent);
    for (auto const& child : particle.getDaughters())
      histograms_.fill("SimParticles.children", child);

    // PN particles are special
    if (particle.getProcessType() == ldmx::SimParticle::ProcessType::photonNuclear) {
      histograms_.fill("pn_child.E", particle.getEnergy());
      histograms_.fill("pn_child.px",momentum.at(0));
      histograms_.fill("pn_child.py",momentum.at(1));
      histograms_.fill("pn_child.pz",momentum.at(2));
      histograms_.fill("pn_child.time",particle.getTime());
      histograms_.fill("pn_child.pdg",particle.getPdgID());
      histograms_.fill("pn_child.x", vertex.at(0));
      histograms_.fill("pn_child.y", vertex.at(1));
      histograms_.fill("pn_child.z", vertex.at(2));
      histograms_.fill("pn_child.track_id", track_id);
      for (auto const& parent : particle.getParents())
        histograms_.fill("pn_child.parent", parent);
      for (auto const& child : particle.getDaughters())
        histograms_.fill("pn_child.children", child);
    }
  }  // loop over sim particle map

  for (auto const& pt : calo_colls) {
    auto const& coll{event.getCollection<ldmx::SimCalorimeterHit>(pt.name(),sim_pass_)};
    for (auto const& hit : coll) {
      unsigned int n_contribs{hit.getNumberOfContribs()};
      histograms_.fill(pt.name()+".n_contribs", n_contribs);
      for (unsigned int i_contrib{0}; i_contrib<n_contribs; i_contrib++) {
        ldmx::SimCalorimeterHit::Contrib contrib{hit.getContrib(i_contrib)}; 
        histograms_.fill(pt.name()+".incidents", contrib.incidentID);
        histograms_.fill(pt.name()+".tracks", contrib.trackID);
        histograms_.fill(pt.name()+".pdg", contrib.pdgCode);
      }

      histograms_.fill(pt.name()+".edep", hit.getEdep());
      auto pos{hit.getPosition()};
      histograms_.fill(pt.name()+".x",pos.at(0));
      histograms_.fill(pt.name()+".y",pos.at(1));
      histograms_.fill(pt.name()+".z",pos.at(2));
      histograms_.fill(pt.name()+".time", hit.getTime());
    }  // loop over hits in the calorimeter collection
  }    // loop over different calorimeter hit collections

  for (auto const& pt : track_colls) {
    auto const& coll{event.getCollection<ldmx::SimTrackerHit>(pt.name(),sim_pass_)};
    for (auto const& hit : coll) {
      histograms_.fill(pt.name()+".particle_E", hit.getEnergy());
      auto momentum{hit.getMomentum()};
      histograms_.fill(pt.name()+".particle_px", momentum.at(0));
      histograms_.fill(pt.name()+".particle_py", momentum.at(1));
      histograms_.fill(pt.name()+".particle_pz", momentum.at(2));
      histograms_.fill(pt.name()+".edep", hit.getEdep());
      histograms_.fill(pt.name()+".time", hit.getTime());
      auto pos{hit.getPosition()};
      histograms_.fill(pt.name()+".x", pos.at(0));
      histograms_.fill(pt.name()+".y", pos.at(1));
      histograms_.fill(pt.name()+".z", pos.at(2));
      histograms_.fill(pt.name()+".track", hit.getTrackID());
      histograms_.fill(pt.name()+".pdg", hit.getPdgID());
    }  // loop over hits in the tracker collection
  }    // loop over different tracker hit collections

  return;
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, SimObjects);
