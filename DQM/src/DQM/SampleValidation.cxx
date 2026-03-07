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
    std::vector<int> parents_track_ids = p.getParents();
    std::vector<int> daughters = p.getDaughters();
    const auto& pdgid = p.getPdgID();
    const auto& vertex = p.getVertex();
    const auto& energy = p.getEnergy();
    const auto& momentum = p.getMomentum();
    for (auto const& parent_track_id : parents_track_ids) {
      if (parent_track_id == 0) {
        ROOT::Math::XYZVector momentum_vec(momentum[0], momentum[1],
                                           momentum[2]);
        histograms_.fill("primaries_pdgid", pdgidLabel(pdgid));
        histograms_.fill("primaries_energy", energy);
        histograms_.fill("primaries_theta",
                         momentum_vec.Theta() * (180 / 3.14159));
        histograms_.fill("primaries_pt", std::sqrt(momentum_vec.Perp2()));
        hard_thresh = (2500. / 4000.) * energy;
        primary_daughters = daughters;
        for (const ldmx::SimTrackerHit& sphit : target_sp_hits) {
          if (sphit.getTrackID() == it.first && sphit.getPosition()[2] < 0) {
            histograms_.fill("beam_smear", vertex[0], vertex[1]);
          }
        }
      }
    }  // end loop over parents
  }  // end loop over SimParticles (1st time)

  std::vector<std::vector<int>> hardbrem_daughters;

  for (auto const& it : particle_map) {
    int trackid = it.first;
    ldmx::SimParticle p = it.second;
    const auto& momentum = p.getMomentum();
    ROOT::Math::XYZVector momentum_vec(momentum[0], momentum[1], momentum[2]);
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
          histograms_.fill("harddaughters_theta",
                           momentum_vec.Theta() * (180 / 3.14159));
          histograms_.fill("harddaughters_pt", std::sqrt(momentum_vec.Perp2()));
          hardbrem_daughters.push_back(p.getDaughters());
        }
      }
    }  // end loop over primary daughters
  }  // end loop over SimParticles (2nd time)

  for (auto const& it : particle_map) {
    int trackid = it.first;
    ldmx::SimParticle p = it.second;
    const auto& momentum = p.getMomentum();
    ROOT::Math::XYZVector momentum_vec(momentum[0], momentum[1], momentum[2]);
    for (const std::vector<int>& daughter_track_id : hardbrem_daughters) {
      for (const int& daughter_id : daughter_track_id) {
        if (trackid == daughter_id) {
          histograms_.fill("hardbremdaughters_pdgid", pdgidLabel(p.getPdgID()));
          histograms_.fill("hardbremdaughters_startZ", p.getVertex()[2]);
          histograms_.fill("hardbremdaughters_endZ", p.getEndPoint()[2]);
          histograms_.fill("hardbremdaughters_energy", p.getEnergy());
          histograms_.fill("hardbremdaughters_theta",
                           momentum_vec.Theta() * (180 / 3.14159));
          histograms_.fill("hardbremdaughters_pt",
                           std::sqrt(momentum_vec.Perp2()));
        }
      }
    }  // end loop over hardbrem daughters
  }  // end loop over SimParticles (3x time)

  return;
}

float SampleValidation::pdgidLabel(const int pdgid) {
  // initially assign label as "anything else"/overflow value,
  // only change if the pdg id is something of interest
  int label = 19;
  if (pdgid == -11) label = 0;    // e+
  if (pdgid == 11) label = 1;     // e-
  if (pdgid == -13) label = 2;    // μ+
  if (pdgid == 13) label = 3;     // μ-
  if (pdgid == 22) label = 4;     // γ
  if (pdgid == 2212) label = 5;   // proton
  if (pdgid == 2112) label = 6;   // neutron
  if (pdgid == 211) label = 7;    // π+
  if (pdgid == -211) label = 8;   // π-
  if (pdgid == 111) label = 9;    // π0
  if (pdgid == 321) label = 10;   // K+
  if (pdgid == -321) label = 11;  // K-
  if (pdgid == 130) label = 12;   // K-Long
  if (pdgid == 310) label = 13;   // K-Short
  if (pdgid == 3122 || pdgid == 3222 || pdgid == 3212 || pdgid == 3112 ||
      pdgid == 3322 || pdgid == 3312) {
    label = 16;  // strange baryons
  }
  /*
   * Nuclear PDG codes are given by ±10LZZZAAAI so to find the atomic
   * number, we divide by 10 (to lose I) and then take the modulo
   * with 1000.
   */
  if (pdgid > 1000000000) {  // nuclei
    if (((pdgid / 10) % 1000) <= 4) {
      label = 14;  // light nuclei
    } else {
      label = 15;  // heavy nuclei
    }
  }
  // dark photon, need pdg id for other models like ALPs and SIMPs
  if (pdgid == 622) label = 17;

  if (pdgid == 17) label = 18;   // fcp-
  if (pdgid == -17) label = 19;  // fcp+
  if (label == 19) {
    ldmx_log(debug) << "Unrecognized PDG ID: " << pdgid
                    << ", assigning to 'else'";
  }

  return label + 0.5;
}

}  // namespace dqm
DECLARE_ANALYZER(dqm::SampleValidation)
