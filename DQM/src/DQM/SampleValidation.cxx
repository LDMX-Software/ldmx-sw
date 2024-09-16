#include "DQM/SampleValidation.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include "Framework/NtupleManager.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"

namespace dqm {

void SampleValidation::configure(framework::config::Parameters& ps) { return; }

void SampleValidation::analyze(const framework::Event& event) {
  // Grab the SimParticle Map and Target Scoring Plane Hits
  auto targetSPHits(
      event.getCollection<ldmx::SimTrackerHit>("TargetScoringPlaneHits"));
  auto particle_map{event.getMap<int, ldmx::SimParticle>("SimParticles")};

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
        histograms_.fill("pdgid_primaries", pdgid_label(pdgid));
        histograms_.fill("energy_primaries", energy);
        hard_thresh = (2500. / 4000.) * energy;
        primary_daughters = daughters;
        for (const ldmx::SimTrackerHit& sphit : targetSPHits) {
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
    for (const std::vector<int>& daughter_track_id : hardbrem_daughters) {
      for (const int& daughter_id : daughter_track_id) {
        if (trackid == daughter_id) {
          histograms_.fill("pdgid_hardbremdaughters",
                           pdgid_label(p.getPdgID()));
          histograms_.fill("startZ_hardbremdaughters", p.getVertex()[2]);
        }
      }
    }
  }

  return;
}

int SampleValidation::pdgid_label(const int pdgid) {
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
  if (pdgid == 211) label = 8;    //π+
  if (pdgid == -211) label = 9;   //π-
  if (pdgid == 111) label = 10;   //π0
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

  return label;
}

void SampleValidation::onProcessStart() {
  std::vector<std::string> labels = {"",
                                     "e^{+}",                   // 1
                                     "e^{-}",                   // 2
                                     "#mu^{+}",                 // 3
                                     "#mu^{-}",                 // 4
                                     "#gamma",                  // 5
                                     "p^{+}",                   // 6
                                     "n^{0}",                   // 7
                                     "#pi^{+}",                 // 8
                                     "#pi^{-}",                 // 9
                                     "#pi^{0}",                 // 10
                                     "K^{+}",                   // 11
                                     "K^{-}",                   // 12
                                     "K_{L}",                   // 13
                                     "K_{S}",                   // 14
                                     "light-N",                 // 15
                                     "heavy-N",                 // 16
                                     "#Lambda / #Sigma / #Xi",  // 17
                                     "A'",                      // 18
                                     "else",
                                     ""};

  std::vector<TH1*> hists = {
      histograms_.get("pdgid_primaries"),
      histograms_.get("pdgid_primarydaughters"),
      histograms_.get("pdgid_harddaughters"),
      histograms_.get("pdgid_hardbremdaughters"),

  };

  for (int ilabel{1}; ilabel < labels.size(); ++ilabel) {
    for (auto& hist : hists) {
      hist->GetXaxis()->SetBinLabel(ilabel, labels[ilabel - 1].c_str());
    }
  }
}
}  // namespace dqm
DECLARE_ANALYZER_NS(dqm, SampleValidation)
