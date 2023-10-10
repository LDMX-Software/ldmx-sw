#include "DQM/SampleValidation.h"
#include "SimCore/Event/SimParticle.h"
#include "Framework/NtupleManager.h"
#include <iostream>
#include <fstream>
#include <algorithm>

namespace dqm {

  void SampleValidation::configure(framework::config::Parameters& ps) {

    return;
  }

  void SampleValidation::analyze(const framework::Event& event) {

    //Grab the SimParticle Map                                                        
    auto particle_map{event.getMap<int, ldmx::SimParticle>("SimParticles")};

    std::vector<int> primary_daughters;

    //Loop over all SimParticles                                                      
    for (auto const& it : particle_map) {
      ldmx::SimParticle p = it.second;
      int pdgid = p.getPdgID();
      std::vector<double> vertex = p.getVertex();
      double energy = p.getEnergy();
      std::vector<int> parents_track_ids = p.getParents();
      std::vector<int> daughters = p.getDaughters();

      for (auto const& parent_track_id: parents_track_ids) {
	if (parent_track_id == 0) {
	  histograms_.fill("beam_smear", vertex[0], vertex[1]);
	  histograms_.fill("pdgid_primaries", pdgid);
	  histograms_.fill("energy_primaries", energy);
	  primary_daughters = daughters;
	}
      }
    }

    std::vector<std::vector<int>> hardbrem_daughters;

    for (auto const& it : particle_map) {
      int trackid = it.first;
      ldmx::SimParticle p = it.second;
      for (auto const& primary_daughter : primary_daughters) {
	if (trackid == primary_daughter) {
	  histograms_.fill("pdgid_primarydaughters", p.getPdgID());
	  if (p.getEnergy() >= 2500) {
	    histograms_.fill("startZ_hardbrem", p.getVertex()[2]);
	    histograms_.fill("endZ_hardbrem", p.getEndPoint()[2]);
	    histograms_.fill("energy_hardbrem", p.getEnergy());
	    hardbrem_daughters.push_back(p.getDaughters());
	  }
	}
      }
    }

    for (auto const& it : particle_map) {
      int trackid = it.first;
      ldmx::SimParticle p = it.second;
      for (const std::vector<int> &daughter_track_id : hardbrem_daughters){
	for (const int &daughter_id : daughter_track_id) {
	  if (trackid == daughter_id) {
	    histograms_.fill("pdgid_hardbremdaughters", p.getPdgID());
	    histograms_.fill("startZ_hardbremdaughters", p.getVertex()[2]);
	  }
	}
      }
    }


    return;
  }
}

DECLARE_ANALYZER_NS(dqm, SampleValidation)
