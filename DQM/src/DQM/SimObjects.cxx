
#include "DQM/SimObjects.h"

#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "SimCore/Event/SimCalorimeterHit.h"

namespace dqm {

void SimObjects::configure(framework::config::Parameters &ps) {
  sim_particles_name_ = ps.getParameter<std::string>("sim_particles_name");
  sim_calorimeter_names_ = ps.getParameter<std::vector<std::string>>("sim_calorimeter_names");
  sim_tracker_names_ = ps.getParameter<std::vector<std::string>>("sim_tracker_names");
  sim_pass_ = ps.getParameter<std::string>("sim_pass");

  return;
}

void SimObjects::onProcessStart() {
  d_base = getHistoDirectory();

  // create sim particles histograms 
  histograms_.create(sim_particles_name_+".E","Vertex Total Energy [MeV]",800,0.,8000.);
  histograms_.create(sim_particles_name_+".px","Vertex Momentum in x-Direction [MeV]",50,0.,500.);
  histograms_.create(sim_particles_name_+".py","Vertex Momentum in y-Direction [MeV]",50,0.,500.);
  histograms_.create(sim_particles_name_+".pz","Vertex Momentum in z-Direction [MeV]",400,0.,4000.);
  histograms_.create(sim_particles_name_+".time","Global Time of Creation [ns]",50,0.,10.);
  histograms_.create(sim_particles_name_+".pdg","PDG ID of Particle",201,-100,100);
  histograms_.create(sim_particles_name_+".x","Vertex x-Position [mm]",401,-200,200);
  histograms_.create(sim_particles_name_+".y","Vertex y-Position [mm]",401,-200,200);
  histograms_.create(sim_particles_name_+".z","Vertex z-Position [mm]",171,-700,1000.);
  histograms_.create(sim_particles_name_+".process","Creator Process Type",12,0,12);
  histograms_.create(sim_particles_name_+".track_id","Track ID of Particle",100,0,1000);
  histograms_.create(sim_particles_name_+".parent","Track ID of Parent",100,0,1000);
  histograms_.create(sim_particles_name_+".children","Track IDs of Children",100,0,1000);

  // calorimeter hit histograms
  for (auto const& coll_name : sim_calorimeter_names_) {
    histograms_.create(coll_name+".incidents","Incident Track IDs",100,0,1000);
    histograms_.create(coll_name+".tracks","Contributing Track IDs",100,0,1000);
    histograms_.create(coll_name+".pdg","Contributing PDG IDs",401,-200,200);
    histograms_.create(coll_name+".edep","Energy Deposited [MeV]",100,0,200);
    histograms_.create(coll_name+".x","Hit x-Position [mm]",401,-200,200);
    histograms_.create(coll_name+".y","Hit y-Position [mm]",401,-200,200);
    histograms_.create(coll_name+".z","Hit z-Position [mm]",171,-700,1000.);
    histograms_.create(coll_name+".time","Hit Time [ns]",50,0.,10.);
    histograms_.create(coll_name+".n_contribs","Number of Contributors",20,0.,20.);
  }  // loop over different calorimeter hit collections

  // tracker hit histograms
  for (auto const& coll_name : sim_tracker_names_) {
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
  }  // loop over different tracker hit collections

}

void SimObjects::analyze(const framework::Event &event) {
  auto const& particle_map{event.getMap<int,ldmx::SimParticle>(sim_particles_name_,sim_pass_)};
  for (auto const& [track_id, particle] : particle_map) {
    auto const& momentum{particle.getMomentum()};
    auto const& vertex{particle.getVertex()};
    histograms_.fill(sim_particles_name_+".E", particle.getEnergy());
    histograms_.fill(sim_particles_name_+".px",momentum.at(0));
    histograms_.fill(sim_particles_name_+".py",momentum.at(1));
    histograms_.fill(sim_particles_name_+".pz",momentum.at(2));
    histograms_.fill(sim_particles_name_+".time",particle.getTime());
    histograms_.fill(sim_particles_name_+".pdg",particle.getPdgID());
    histograms_.fill(sim_particles_name_+".x", vertex.at(0));
    histograms_.fill(sim_particles_name_+".y", vertex.at(1));
    histograms_.fill(sim_particles_name_+".z", vertex.at(2));
    histograms_.fill(sim_particles_name_+".process", particle.getProcessType());
    histograms_.fill(sim_particles_name_+".track_id", track_id);
    for (auto const& parent : particle.getParents())
      histograms_.fill(sim_particles_name_+".parent", parent);
    for (auto const& child : particle.getDaughters())
      histograms_.fill(sim_particles_name_+".children", child);
  }  // loop over sim particle map

  for (auto const& coll_name : sim_calorimeter_names_) {
    if (not event.exists(coll_name,sim_pass_)) {
      ldmx_log(warn) << "'" << coll_name << "' of pass '" << sim_pass_ << "' not found!.";
      continue;
    }

    auto const& coll{event.getCollection<ldmx::SimCalorimeterHit>(coll_name,sim_pass_)};
    for (auto const& hit : coll) {
      int n_contribs{hit.getNumberOfContribs()};
      histograms_.fill(coll_name+".n_contribs", n_contribs);
      for (int i_contrib{0}; i_contrib<n_contribs; i_contrib++) {
        ldmx::SimCalorimeterHit::Contrib contrib{hit.getContrib(i_contrib)}; 
        histograms_.fill(coll_name+".incidents", contrib.incidentID);
        histograms_.fill(coll_name+".tracks", contrib.trackID);
        histograms_.fill(coll_name+".pdg", contrib.pdgCode);
      }

      histograms_.fill(coll_name+".edep", hit.getEdep());
      auto pos{hit.getPosition()};
      histograms_.fill(coll_name+".x",pos.at(0));
      histograms_.fill(coll_name+".y",pos.at(1));
      histograms_.fill(coll_name+".z",pos.at(2));
      histograms_.fill(coll_name+".time", hit.getTime());
    }  // loop over hits in the calorimeter collection
  }    // loop over different calorimeter hit collections

  for (auto const& coll_name : sim_tracker_names_) {
    if (not event.exists(coll_name,sim_pass_)) {
      ldmx_log(warn) << "'" << coll_name << "' of pass '" << sim_pass_ << "' not found!.";
      continue;
    }

    auto const& coll{event.getCollection<ldmx::SimTrackerHit>(coll_name,sim_pass_)};
    for (auto const& hit : coll) {
      histograms_.fill(coll_name+".particle_E", hit.getEnergy());
      auto momentum{hit.getMomentum()};
      histograms_.fill(coll_name+".particle_px", momentum.at(0));
      histograms_.fill(coll_name+".particle_py", momentum.at(1));
      histograms_.fill(coll_name+".particle_pz", momentum.at(2));
      histograms_.fill(coll_name+".edep", hit.getEdep());
      histograms_.fill(coll_name+".time", hit.getTime());
      auto pos{hit.getPosition()};
      histograms_.fill(coll_name+".x", pos.at(0));
      histograms_.fill(coll_name+".y", pos.at(1));
      histograms_.fill(coll_name+".z", pos.at(2));
      histograms_.fill(coll_name+".track", hit.getTrackID());
      histograms_.fill(coll_name+".pdg", hit.getPdgID());
    }  // loop over hits in the tracker collection
  }    // loop over different tracker hit collections

  return;
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, SimObjects);
