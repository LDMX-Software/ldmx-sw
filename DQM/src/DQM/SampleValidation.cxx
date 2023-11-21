#include "DQM/SampleValidation.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "Framework/NtupleManager.h"
#include <iostream>
#include <fstream>
#include <algorithm>

namespace dqm {

  void SampleValidation::configure(framework::config::Parameters& ps) {

    return;
  }

  void SampleValidation::analyze(const framework::Event& event) {

    //Grab the SimParticle Map and Target Scoring Plane Hits
    auto targetSPHits(event.getCollection<ldmx::SimTrackerHit>("TargetScoringPlaneHits"));
    auto particle_map{event.getMap<int, ldmx::SimParticle>("SimParticles")};

    std::vector<int> primary_daughters;

    double hard_thresh = 2500;

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
	  histograms_.fill("pdgid_primaries", pdgid_label(pdgid));
	  histograms_.fill("energy_primaries", energy);
	  hard_thresh = (2500/4000)*energy;
	  primary_daughters = daughters;
	  for (const ldmx::SimTrackerHit &sphit : targetSPHits) {
	    if (sphit.getTrackID() == it.first && sphit.getPosition()[2] < 0) {
	      histograms_.fill("beam_smear", vertex[0], vertex[1]);
	    }
	  }
	}
      }
    }

    std::vector<std::vector<int>> hardbrem_daughters;

    for (auto const& it : particle_map) {
      int trackid = it.first;
      ldmx::SimParticle p = it.second;
      for (auto const& primary_daughter : primary_daughters) {
	if (trackid == primary_daughter) {
	  histograms_.fill("pdgid_primarydaughters", pdgid_label(p.getPdgID()));
	  if (p.getPdgID() == 22) {
	    histograms_.fill("energy_daughterphoton", p.getEnergy());
	  }
	  if (p.getEnergy() >= hard_thresh) {
	    histograms_.fill("pdgid_harddaughters", pdgid_label(p.getPdgID()));
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
	    histograms_.fill("pdgid_hardbremdaughters", pdgid_label(p.getPdgID()));
	    histograms_.fill("startZ_hardbremdaughters", p.getVertex()[2]);
	  }
	}
      }
    }


    return;
  }

  int SampleValidation::pdgid_label(const int pdgid) {
    int label = 18; // initially assign label as "anything else"/overflow value, only change if the pdg id is something of interest
    if (pdgid == -11) label = 1; // e+
    
    if (pdgid == 11) label = 2; // e-
    
    if (pdgid == -13) label = 3; // μ+
    
    if (pdgid == 13) label = 4; // μ-
    
    if (pdgid == 22) label = 5; // γ
    
    if (pdgid == 2212) label = 6; // proton
    
    if (pdgid == 2112) label = 7; // neutron
    
    if (pdgid == 211) label = 8; //π+
    
    if (pdgid == -211) label = 9; //π-
    
    if (pdgid == 111) label = 10; //π0
    
    if (pdgid == 321) label = 11; // K+
    
    if (pdgid == -321) label = 12; // K-
    
    if (pdgid == 130) label = 13; // K-Long
    
    if (pdgid == 310) label = 14; // K-Short
    
    if (pdgid == 3122 || pdgid == 3222 || pdgid == 3212 || pdgid == 3112 || pdgid == 3322 || pdgid == 3312) label = 16; // strange baryon
    
    if (pdgid > 10000) label = 15; //nuclei

    if (pdgid == 622) label = 17; // dark photon, need pdg id for other models like ALPs and SIMPs
    
    return label;
  }

}

DECLARE_ANALYZER_NS(dqm, SampleValidation)
