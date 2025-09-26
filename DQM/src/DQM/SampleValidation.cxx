#include "DQM/SampleValidation.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include "Framework/NtupleManager.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"

namespace dqm {

void SampleValidation::configure(framework::config::Parameters& ps) {
  target_scoring_plane_passname_ =
      ps.get<std::string>("target_scoring_plane_passname");
  sim_particles_passname_ = ps.get<std::string>("sim_particles_passname");
}

void SampleValidation::analyze(const framework::Event& event) {
  // Grab the SimParticle Map and Target Scoring Plane Hits
  auto target_sp_hits(event.getCollection<ldmx::SimTrackerHit>(
      "TargetScoringPlaneHits", target_scoring_plane_passname_));
  auto particle_map{event.getMap<int, ldmx::SimParticle>(
      "SimParticles", sim_particles_passname_)};

  std::vector<int> primary_daughters;

  double hard_thresh{9999.0};

  // Loop over all SimParticles
  for (auto const& it : particle_map) {
    ldmx::SimParticle p = it.second;
    int pdgid = p.getPdgID();
    std::vector<double> vertex = p.getVertex();
    double energy = p.getEnergy();
    std::vector<int> parents_track_ids = p.getParents();
    std::vector<int> daughters = p.getDaughters();

    for (auto const& parent_track_id : parents_track_ids) {
      if (parent_track_id == 0) {
        histograms_.fill("primaries_pdgid", pdgidLabel(pdgid));
        histograms_.fill("primaries_energy", energy);
        hard_thresh = (2500. / 4000.) * energy;
        primary_daughters = daughters;
        for (const ldmx::SimTrackerHit& sphit : target_sp_hits) {
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
        histograms_.fill("primarydaughters_pdgid", pdgidLabel(p.getPdgID()));
        if (p.getPdgID() == 22) {
          histograms_.fill("daughterphoton_energy", p.getEnergy());
        }
        if (p.getEnergy() >= hard_thresh) {
          histograms_.fill("harddaughters_pdgid", pdgidLabel(p.getPdgID()));
          histograms_.fill("harddaughters_startZ", p.getVertex()[2]);
          histograms_.fill("harddaughters_endZ", p.getEndPoint()[2]);
          histograms_.fill("harddaughters_energy", p.getEnergy());
          hardbrem_daughters.push_back(p.getDaughters());
        }
      }
    }
  }

  for (auto const& it : particle_map) {
    int trackid = it.first;
    ldmx::SimParticle p = it.second;
    for (const std::vector<int>& daughter_track_id : hardbrem_daughters) {
      for (const int& daughter_id : daughter_track_id) {
        if (trackid == daughter_id) {
          histograms_.fill("hardbremdaughters_pdgid", pdgidLabel(p.getPdgID()));
          histograms_.fill("hardbremdaughters_startZ", p.getVertex()[2]);
          histograms_.fill("hardbremdaughters_endZ", p.getEndPoint()[2]);
          histograms_.fill("hardbremdaughters_energy", p.getEnergy());
        }
      }
    }
  }

  return;
}

int SampleValidation::pdgidLabel(const int pdgid) {
  // initially assign label as "anything else"/overflow value,
  // only change if the pdg id is something of interest
  int label = 19;
  if (pdgid == -11) label = 1;    // e+
  if (pdgid == 11) label = 2;     // e-
  if (pdgid == -13) label = 3;    // μ+
  if (pdgid == 13) label = 4;     // μ-
  if (pdgid == 22) label = 5;     // γ
  if (pdgid == 2212) label = 6;   // proton
  if (pdgid == 2112) label = 7;   // neutron
  if (pdgid == 211) label = 8;    // π+
  if (pdgid == -211) label = 9;   // π-
  if (pdgid == 111) label = 10;   // π0
  if (pdgid == 321) label = 11;   // K+
  if (pdgid == -321) label = 12;  // K-
  if (pdgid == 130) label = 13;   // K-Long
  if (pdgid == 310) label = 14;   // K-Short
  if (pdgid == 3122 || pdgid == 3222 || pdgid == 3212 || pdgid == 3112 ||
      pdgid == 3322 || pdgid == 3312)
    label = 17;  // strange baryon
  /*
   * Nuclear PDG codes are given by ±10LZZZAAAI so to find the atomic
   * number, we divide by 10 (to lose I) and then take the modulo
   * with 1000.
   */
  if (pdgid > 1000000000) {  // nuclei
    if (((pdgid / 10) % 1000) <= 4) {
      label = 15;  // light nuclei
    } else {
      label = 16;  // heavy nuclei
    }
  }
  if (pdgid == 622)
    label =
        18;  // dark photon, need pdg id for other models like ALPs and SIMPs

  return label - 1;
}

}  // namespace dqm
DECLARE_ANALYZER(dqm::SampleValidation)
