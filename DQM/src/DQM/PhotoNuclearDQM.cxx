
#include "DQM/PhotoNuclearDQM.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <algorithm>

//----------//
//   ROOT   //
//----------//
#include "TH1F.h"
#include "TH2F.h"

//----------//
//   LDMX   //
//----------//
#include "Framework/Event.h"
#include "Tools/AnalysisUtils.h"

#include <TVector3.h>

namespace dqm {

PhotoNuclearDQM::PhotoNuclearDQM(const std::string &name,
                                 framework::Process &process)
    : framework::Analyzer(name, process) {}

PhotoNuclearDQM::~PhotoNuclearDQM() {}
std::vector<const ldmx::SimParticle *> PhotoNuclearDQM::findDaughters(
    const std::map<int, ldmx::SimParticle> particleMap,
    const ldmx::SimParticle *parent) const {
  std::vector<const ldmx::SimParticle *> pnDaughters;
  for (const auto &daughterTrackID : parent->getDaughters()) {
    // skip daughters that weren't saved
    if (particleMap.count(daughterTrackID) == 0) {
      continue;
    }

    auto daughter{&(particleMap.at(daughterTrackID))};

    // Get the PDG ID
    auto pdgID{daughter->getPdgID()};

    // Ignore photons and nuclei
    if (pdgID == 22 ||
        (pdgID > 10000 && (!count_light_ions_ || !isLightIon(pdgID)))) {
      continue;
    }
    pnDaughters.push_back(daughter);
  }

  std::sort(pnDaughters.begin(), pnDaughters.end(),
            [](const auto &lhs, const auto &rhs) {
              double lhs_ke = lhs->getEnergy() - lhs->getMass();
              double rhs_ke = rhs->getEnergy() - rhs->getMass();
              return lhs_ke > rhs_ke;
            });

  return pnDaughters;
}
void PhotoNuclearDQM::findRecoilProperties(const ldmx::SimParticle *recoil) {

  histograms_.fill("recoil_vertex_x", recoil->getVertex()[0]);
  histograms_.fill("recoil_vertex_y", recoil->getVertex()[1]);
  histograms_.fill("recoil_vertex_z", recoil->getVertex()[2]);
  histograms_.fill("recoil_vertex_x:recoil_vertex_y", recoil->getVertex()[0],
                   recoil->getVertex()[1]);
}
void PhotoNuclearDQM::findParticleKinematics(
    const std::vector<const ldmx::SimParticle *> &pnDaughters) {
  double hardest_ke{-1}, hardest_theta{-1};
  double hardest_proton_ke{-1}, hardest_proton_theta{-1};
  double hardest_neutron_ke{-1}, hardest_neutron_theta{-1};
  double hardest_pion_ke{-1}, hardest_pion_theta{-1};
  double total_ke{0};
  double total_neutron_ke{0};
  int neutron_multiplicity{0};
  // Loop through all of the PN daughters and extract kinematic
  // information.
  for (const auto *daughter : pnDaughters) {
    // skip daughters that weren't saved

    // Get the PDG ID
    auto pdgID{daughter->getPdgID()};

    // Calculate the kinetic energy
    double ke{daughter->getEnergy() - daughter->getMass()};
    total_ke += ke;

    std::vector<double> vec{daughter->getMomentum()};
    TVector3 pvec(vec[0], vec[1], vec[2]);

    //  Calculate the polar angle
    auto theta{pvec.Theta() * (180 / 3.14159)};

    if (hardest_ke < ke) {
      hardest_ke = ke;
      hardest_theta = theta;
    }

    if ((pdgID == 2112)) {
      total_neutron_ke += ke;
      neutron_multiplicity++;
      if (hardest_neutron_ke < ke) {
        hardest_neutron_ke = ke;
        hardest_neutron_theta = theta;
      }
    }

    if ((pdgID == 2212) && (hardest_proton_ke < ke)) {
      hardest_proton_ke = ke;
      hardest_proton_theta = theta;
    }

    if (((std::abs(pdgID) == 211) || (pdgID == 111)) &&
        (hardest_pion_ke < ke)) {
      hardest_pion_ke = ke;
      hardest_pion_theta = theta;
    }
  }
  histograms_.fill("hardest_ke", hardest_ke);
  histograms_.fill("hardest_theta", hardest_theta);
  histograms_.fill("h_ke_h_theta", hardest_ke, hardest_theta);
  histograms_.fill("hardest_p_ke", hardest_proton_ke);
  histograms_.fill("hardest_p_theta", hardest_proton_theta);
  histograms_.fill("hardest_n_ke", hardest_neutron_ke);
  histograms_.fill("hardest_n_theta", hardest_neutron_theta);
  histograms_.fill("hardest_pi_ke", hardest_pion_ke);
  histograms_.fill("hardest_pi_theta", hardest_pion_theta);

  histograms_.fill("pn_neutron_mult", neutron_multiplicity);
  histograms_.fill("pn_total_ke", total_ke);
  histograms_.fill("pn_total_neutron_ke", total_neutron_ke);
}

void PhotoNuclearDQM::findSubleadingKinematics(
    const ldmx::SimParticle *pnGamma,
    const std::vector<const ldmx::SimParticle *> &pnDaughters,
    const PhotoNuclearDQM::EventType eventType) {

  // Note: Assumes sorted by energy

  double subleading_ke{-9999};
  double nEnergy{-9999}, energyDiff{-9999}, energyFrac{-9999};

  nEnergy = pnDaughters[0]->getEnergy() - pnDaughters[0]->getMass();
  subleading_ke = -9999;
  if (pnDaughters.size() > 1) {
    subleading_ke = pnDaughters[1]->getEnergy() - pnDaughters[1]->getMass();
  }
  energyDiff = pnGamma->getEnergy() - nEnergy;
  energyFrac = nEnergy / pnGamma->getEnergy();

  if (eventType == EventType::single_neutron) {
    histograms_.fill("1n_ke:2nd_h_ke", nEnergy, subleading_ke);
    histograms_.fill("1n_neutron_energy", nEnergy);
    histograms_.fill("1n_energy_diff", energyDiff);
    histograms_.fill("1n_energy_frac", energyFrac);
  } else if (eventType == EventType::two_neutrons) {
    histograms_.fill("2n_n2_energy", subleading_ke);
    auto energyFrac2n = (nEnergy + subleading_ke) / pnGamma->getEnergy();
    histograms_.fill("2n_energy_frac", energyFrac2n);
    histograms_.fill("2n_energy_other", pnGamma->getEnergy() - energyFrac2n);

  } else if (eventType == EventType::charged_kaon) {
    histograms_.fill("1kp_ke:2nd_h_ke", nEnergy, subleading_ke);
    histograms_.fill("1kp_energy", nEnergy);
    histograms_.fill("1kp_energy_diff", energyDiff);
    histograms_.fill("1kp_energy_frac", energyFrac);
  } else if (eventType == EventType::klong || eventType == EventType::kshort) {
    histograms_.fill("1k0_ke:2nd_h_ke", nEnergy, subleading_ke);
    histograms_.fill("1k0_energy", nEnergy);
    histograms_.fill("1k0_energy_diff", energyDiff);
    histograms_.fill("1k0_energy_frac", energyFrac);
  }
}
void PhotoNuclearDQM::onProcessStart() {
  std::vector<std::string> labels = {"",
                                     "Nothing hard",  // 0
                                     "1 n",           // 1
                                     "2 n",           // 2
                                     "#geq 3 n",      // 3
                                     "1 #pi",         // 4
                                     "2 #pi",         // 5
                                     "1 #pi_{0}",     // 6
                                     "1 #pi A",       // 7
                                     "1 #pi 2 A",     // 8
                                     "2 #pi A",       // 9
                                     "1 #pi_{0} A",   // 10
                                     "1 #pi_{0} 2 A", // 11
                                     "#pi_{0} #pi A", // 12
                                     "1 p",           // 13
                                     "2 p",           // 14
                                     "pn",            // 15
                                     "K^{0}_{L} X",   // 16
                                     "K X",           // 17
                                     "K^{0}_{S} X",   // 18
                                     "exotics",       // 19
                                     "multi-body",    // 20
                                     ""};

  std::vector<TH1 *> hists = {
      histograms_.get("event_type"),
      histograms_.get("event_type_500mev"),
      histograms_.get("event_type_2000mev"),

  };

  for (int ilabel{1}; ilabel < labels.size(); ++ilabel) {
    for (auto &hist : hists) {
      hist->GetXaxis()->SetBinLabel(ilabel, labels[ilabel - 1].c_str());
    }
  }

  labels = {"",
            "1 n",     // 0
            "K#pm X",  // 1
            "1 K^{0}", // 2
            "2 n",     // 3
            "Soft",    // 4
            "Other",   // 5
            ""};

  hists = {
      histograms_.get("event_type_compact"),
      histograms_.get("event_type_compact_500mev"),
      histograms_.get("event_type_compact_2000mev"),
  };

  for (int ilabel{1}; ilabel < labels.size(); ++ilabel) {
    for (auto &hist : hists) {
      hist->GetXaxis()->SetBinLabel(ilabel, labels[ilabel - 1].c_str());
    }
  }

  std::vector<std::string> n_labels = {"",
                                       "nn",       // 0
                                       "pn",       // 1
                                       "#pi^{+}n", // 2
                                       "#pi^{0}n", // 3
                                       "other",    // 4
                                       ""};

  TH1 *hist = histograms_.get("1n_event_type");
  for (int ilabel{1}; ilabel < n_labels.size(); ++ilabel) {
    hist->GetXaxis()->SetBinLabel(ilabel, n_labels[ilabel - 1].c_str());
  }
}

void PhotoNuclearDQM::configure(framework::config::Parameters &parameters) {
  verbose_ = parameters.getParameter<bool>("verbose");
  count_light_ions_ = parameters.getParameter<bool>("count_light_ions", true);
}

void PhotoNuclearDQM::analyze(const framework::Event &event) {
  // Get the particle map from the event.  If the particle map is empty,
  // don't process the event.
  auto particleMap{event.getMap<int, ldmx::SimParticle>("SimParticles")};
  if (particleMap.size() == 0) {
    return;
  }

  // Get the recoil electron
  auto [trackID, recoil] = Analysis::getRecoil(particleMap);
  findRecoilProperties(recoil);

  // Use the recoil electron to retrieve the gamma that underwent a
  // photo-nuclear reaction.
  auto pnGamma{Analysis::getPNGamma(particleMap, recoil, 2500.)};
  if (pnGamma == nullptr) {
    if (verbose_) {
      std::cout << "[ PhotoNuclearDQM ]: PN Daughter is lost, skipping."
                << std::endl;
    }
    return;
  }
  const auto pnDaughters{findDaughters(particleMap, pnGamma)};
  findParticleKinematics(pnDaughters);

  histograms_.fill("pn_particle_mult", pnGamma->getDaughters().size());
  histograms_.fill("pn_gamma_energy", pnGamma->getEnergy());
  histograms_.fill("pn_gamma_int_z", pnGamma->getEndPoint()[2]);
  histograms_.fill("pn_gamma_vertex_x", pnGamma->getVertex()[0]);
  histograms_.fill("pn_gamma_vertex_y", pnGamma->getVertex()[1]);
  histograms_.fill("pn_gamma_vertex_z", pnGamma->getVertex()[2]);

  // Classify the event
  auto eventType{classifyEvent(pnDaughters, 200)};
  auto eventType500MeV{classifyEvent(pnDaughters, 500)};
  auto eventType2000MeV{classifyEvent(pnDaughters, 2000)};

  auto eventTypeComp{classifyCompactEvent(pnGamma, pnDaughters, 200)};
  auto eventTypeComp500MeV{classifyCompactEvent(pnGamma, pnDaughters, 500)};
  auto eventTypeComp2000MeV{classifyCompactEvent(pnGamma, pnDaughters, 2000)};


  histograms_.fill("event_type", static_cast<int>(eventType));
  histograms_.fill("event_type_500mev", static_cast<int>(eventType500MeV));
  histograms_.fill("event_type_2000mev", static_cast<int>(eventType2000MeV));

  histograms_.fill("event_type_compact", static_cast<int>(eventTypeComp));
  histograms_.fill("event_type_compact_500mev",
                   static_cast<int>(eventTypeComp500MeV));
  histograms_.fill("event_type_compact_2000mev",
                   static_cast<int>(eventTypeComp2000MeV));

  switch (eventType) {
  case EventType::single_neutron:
    if (eventType == EventType::single_neutron) {
      if (pnDaughters.size() > 1) {
        auto secondHardestPdgID{abs(pnDaughters[1]->getPdgID())};
        auto nEventType{-10};
        if (secondHardestPdgID == 2112) {
          nEventType = 0; // n + n
        } else if (secondHardestPdgID == 2212) {
          nEventType = 1; // p + n
        } else if (secondHardestPdgID == 211) {
          nEventType = 2; // Pi+/- + n
        } else if (secondHardestPdgID == 111) {
          nEventType = 3; // Pi0 + n
        } else {
          nEventType = 4; // other
        }
        histograms_.fill("1n_event_type", nEventType);
      }
    }
    [[fallthrough]]; // Remaining code is important for 1n as well
  case EventType::two_neutrons:
  case EventType::charged_kaon:
  case EventType::klong:
  case EventType::kshort:
    findSubleadingKinematics(pnGamma, pnDaughters, eventType);
    break;
  }
}

PhotoNuclearDQM::EventType PhotoNuclearDQM::classifyEvent(
    const std::vector<const ldmx::SimParticle *> daughters, double threshold) {
  short n{0}, p{0}, pi{0}, pi0{0}, exotic{0}, k0l{0}, kp{0}, k0s{0}, lambda{0};

  // Loop through all of the PN daughters and extract kinematic
  // information.
  for (const auto &daughter : daughters) {
    // Calculate the kinetic energy
    auto ke{daughter->getEnergy() - daughter->getMass()};

    // Assuming the daughters are sorted by kinetic energy, if the kinetic
    // energy is below threshold, we don't need to look at any further
    // particles.
    if (ke <= threshold) {
      break;
    }

    // Get the PDG ID
    auto pdgID{abs(daughter->getPdgID())};

    if (pdgID == 2112) {
      n++;
    } else if (pdgID == 2212) {
      p++;
    } else if (pdgID == 211) {
      pi++;
    } else if (pdgID == 111) {
      pi0++;
    } else if (pdgID == 130) {
      k0l++;
    } else if (pdgID == 321) {
      kp++;
    } else if (pdgID == 310) {
      k0s++;
    } else {
      exotic++;
    }
  }

  int kaons = k0l + kp + k0s;
  int nucleons = n + p;
  int pions = pi + pi0;
  int count = nucleons + pions + exotic + kaons;

  if (count == 0) {
    return EventType::nothing_hard;
  }
  if (count == 1) {
    if (n == 1) {
      return EventType::single_neutron;
    } else if (p == 1) {
      return EventType::single_proton;
    } else if (pi0 == 1) {
      return EventType::single_neutral_pion;
    } else if (pi == 1) {
      return EventType::single_charged_pion;
    }
  }
  if (count == 2) {
    if (n == 2) {
      return EventType::two_neutrons;
    } else if (n == 1 && p == 1) {
      return EventType::proton_neutron;
    } else if (p == 2) {
      return EventType::two_protons;
    } else if (pi == 2) {
      return EventType::two_charged_pions;
    } else if (pi == 1 && nucleons == 1) {
      return EventType::single_charged_pion_and_nucleon;
    } else if (pi0 == 1 && nucleons == 1) {
      return EventType::single_neutral_pion_and_nucleon;
    }
  }

  if (count == 3) {
    if (pi == 1 && nucleons == 2) {
      return EventType::single_charged_pion_and_two_nucleons;
    } else if (pi == 2 && nucleons == 1) {
      return EventType::two_charged_pions_and_nucleon;
    } // else
    else if (pi0 == 1 && nucleons == 2) {
      return EventType::single_neutral_pion_and_two_nucleons;
    } else if (pi0 == 1 && nucleons == 1 && pi == 1) {
      return EventType::single_neutral_pion_charged_pion_and_nucleon;
    }
  }
  if (count >= 3 && count == n) {
    return EventType::three_or_more_neutrons;
  }

  if (kaons == 1) {
    if (k0l == 1) {
      return EventType::klong;
    } else if (kp == 1) {
      return EventType::charged_kaon;
    } else if (k0s == 1) {
      return EventType::kshort;
    }
  }
  if (exotic == count && count != 0) {
    return EventType::exotics;
  }

  // TODO Remove, broken
  if (pi0 == 1) {
    if ((pi == 1) && ((p == 1) || (n == 1)) && (kaons == 0 && exotic == 0)) {
      return EventType::single_neutral_pion_charged_pion_and_nucleon;
    }
  }

  return EventType::multibody;
}

PhotoNuclearDQM::CompactEventType PhotoNuclearDQM::classifyCompactEvent(
    const ldmx::SimParticle *pnGamma,
    const std::vector<const ldmx::SimParticle *> daughters, double threshold) {
  short n{0}, n_t{0}, k0l{0}, kp{0}, k0s{0}, soft{0};

  // Loop through all of the PN daughters and extract kinematic
  // information.
  for (const auto &daughter : daughters) {
    // Calculate the kinetic energy
    auto ke{daughter->getEnergy() - daughter->getMass()};

    // Get the PDG ID
    auto pdgID{abs(daughter->getPdgID())};

    if (ke < 500) {
      soft++;
      continue;
    }

    if (ke >= 0.8 * pnGamma->getEnergy()) {
      if (pdgID == 2112) {
        n++;
      } else if (pdgID == 130) {
        k0l++;
      } else if (pdgID == 321) {
        kp++;
      } else if (pdgID == 310) {
        k0s++;
      }
      continue;
    }

    if ((pdgID == 2112) && ke > threshold) {
      n_t++;
    }
  }

  int neutral_kaons{k0l + k0s};

  if (n != 0) {
    return PhotoNuclearDQM::CompactEventType::single_neutron;
  }
  if (kp != 0) {
    return PhotoNuclearDQM::CompactEventType::single_charged_kaon;
  }
  if (neutral_kaons != 0) {
    return PhotoNuclearDQM::CompactEventType::single_neutral_kaon;
  }
  if (n_t == 2) {
    return PhotoNuclearDQM::CompactEventType::two_neutrons;
  }
  if (soft == daughters.size()) {
    return PhotoNuclearDQM::CompactEventType::soft;
  }

  return PhotoNuclearDQM::CompactEventType::other;
}

} // namespace dqm

DECLARE_ANALYZER_NS(dqm, PhotoNuclearDQM)
