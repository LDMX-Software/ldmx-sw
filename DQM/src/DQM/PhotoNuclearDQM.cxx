
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
#include "TVector3.h"

//----------//
//   LDMX   //
//----------//
#include "Framework/Event.h"
#include "Tools/AnalysisUtils.h"

namespace dqm {

PhotoNuclearDQM::PhotoNuclearDQM(const std::string &name,
                                 framework::Process &process)
    : framework::Analyzer(name, process) {}

PhotoNuclearDQM::~PhotoNuclearDQM() {}
std::vector<const ldmx::SimParticle *> PhotoNuclearDQM::findPNDaughters(
    const std::map<int, ldmx::SimParticle> particleMap,
    const ldmx::SimParticle *pnGamma) const {
  std::vector<const ldmx::SimParticle *> pnDaughters;
  for (const auto &daughterTrackID : pnGamma->getDaughters()) {
    // skip daughters that weren't saved
    if (particleMap.count(daughterTrackID) == 0) {
      continue;
    }

    auto daughter{&(particleMap.at(daughterTrackID))};

    // Get the PDG ID
    auto pdgID{daughter->getPdgID()};

    // Ignore photons and nuclei
    if (pdgID == 22 || pdgID > 10000)
      continue;
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

  std::vector<std::string> n_labels = {"",         "",
                                       "nn",       // 1
                                       "pn",       // 2
                                       "#pi^{+}n", // 3
                                       "#pi^{0}n", // 4
                                       ""};

  TH1 *hist = histograms_.get("1n_event_type");
  for (int ilabel{1}; ilabel < n_labels.size(); ++ilabel) {
    hist->GetXaxis()->SetBinLabel(ilabel, n_labels[ilabel - 1].c_str());
  }
}

void PhotoNuclearDQM::configure(framework::config::Parameters &parameters) {
  verbose_ = parameters.getParameter<bool>("verbose");
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

  histograms_.fill("recoil_vertex_x", recoil->getVertex()[0]);
  histograms_.fill("recoil_vertex_y", recoil->getVertex()[1]);
  histograms_.fill("recoil_vertex_z", recoil->getVertex()[2]);
  histograms_.fill("recoil_vertex_x:recoil_vertex_y", recoil->getVertex()[0],
                   recoil->getVertex()[1]);

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

  histograms_.fill("pn_particle_mult", pnGamma->getDaughters().size());
  histograms_.fill("pn_gamma_energy", pnGamma->getEnergy());
  histograms_.fill("pn_gamma_int_z", pnGamma->getEndPoint()[2]);
  histograms_.fill("pn_gamma_vertex_x", pnGamma->getVertex()[0]);
  histograms_.fill("pn_gamma_vertex_y", pnGamma->getVertex()[1]);
  histograms_.fill("pn_gamma_vertex_z", pnGamma->getVertex()[2]);

  double leading_ke{-1}, leading_theta{-1};
  double leading_proton_ke{-1}, leading_proton_theta{-1};
  double leading_neutron_ke{-1}, leading_neutron_theta{-1};
  double leading_pion_ke{-1}, leading_pion_theta{-1};
  std::vector<const ldmx::SimParticle *> pnDaughters;

  // Loop through all of the PN daughters and extract kinematic
  // information.
  for (const auto &daughterTrackID : pnGamma->getDaughters()) {
    // skip daughters that weren't saved
    if (particleMap.count(daughterTrackID) == 0) {
      continue;
    }

    auto daughter{&(particleMap.at(daughterTrackID))};

    // Get the PDG ID
    auto pdgID{daughter->getPdgID()};

    // Ignore photons and nuclei
    if (pdgID == 22 || pdgID > 10000)
      continue;

    // Calculate the kinetic energy
    double ke{daughter->getEnergy() - daughter->getMass()};

    std::vector<double> vec{daughter->getMomentum()};
    TVector3 pvec(vec[0], vec[1], vec[2]);

    //  Calculate the polar angle
    auto theta{pvec.Theta() * (180 / 3.14159)};

    if (leading_ke < ke) {
      leading_ke = ke;
      leading_theta = theta;
    }

    if ((pdgID == 2112) && (leading_neutron_ke < ke)) {
      leading_neutron_ke = ke;
      leading_neutron_theta = theta;
    }

    if ((pdgID == 2212) && (leading_proton_ke < ke)) {
      leading_proton_ke = ke;
      leading_proton_theta = theta;
    }

    if (((abs(pdgID) == 211) || (pdgID == 111)) && (leading_pion_ke < ke)) {
      leading_pion_ke = ke;
      leading_pion_theta = theta;
    }

    pnDaughters.push_back(daughter);
  }

  histograms_.fill("hardest_ke", leading_ke);
  histograms_.fill("hardest_theta", leading_theta);
  histograms_.fill("h_ke_h_theta", leading_ke, leading_theta);
  histograms_.fill("hardest_p_ke", leading_proton_ke);
  histograms_.fill("hardest_p_theta", leading_proton_theta);
  histograms_.fill("hardest_n_ke", leading_neutron_ke);
  histograms_.fill("hardest_n_theta", leading_neutron_theta);
  histograms_.fill("hardest_pi_ke", leading_pion_ke);
  histograms_.fill("hardest_pi_theta", leading_pion_theta);

  // Classify the event
  auto eventType{classifyEvent(pnDaughters, 200)};
  auto eventType500MeV{classifyEvent(pnDaughters, 500)};
  auto eventType2000MeV{classifyEvent(pnDaughters, 2000)};

  auto eventTypeComp{classifyCompactEvent(pnGamma, pnDaughters, 200)};
  auto eventTypeComp500MeV{classifyCompactEvent(pnGamma, pnDaughters, 500)};
  auto eventTypeComp2000MeV{classifyCompactEvent(pnGamma, pnDaughters, 2000)};

  histograms_.fill("event_type", eventType);
  histograms_.fill("event_type_500mev", eventType500MeV);
  histograms_.fill("event_type_2000mev", eventType2000MeV);

  histograms_.fill("event_type_compact", eventTypeComp);
  histograms_.fill("event_type_compact_500mev", eventTypeComp500MeV);
  histograms_.fill("event_type_compact_2000mev", eventTypeComp2000MeV);

  double subleading_ke{-9999};
  double nEnergy{-9999}, energyDiff{-9999}, energyFrac{-9999};

  if (eventType == 1 || eventType == 17 || eventType == 16 || eventType == 18 ||
      eventType == 2) {
    std::sort(pnDaughters.begin(), pnDaughters.end(),
              [](const auto &lhs, const auto &rhs) {
                double lhs_ke = lhs->getEnergy() - lhs->getMass();
                double rhs_ke = rhs->getEnergy() - rhs->getMass();
                return lhs_ke > rhs_ke;
              });

    nEnergy = pnDaughters[0]->getEnergy() - pnDaughters[0]->getMass();
    subleading_ke = -9999;
    if (pnDaughters.size() > 1) {
      subleading_ke = pnDaughters[1]->getEnergy() - pnDaughters[1]->getMass();
    }
    energyDiff = pnGamma->getEnergy() - nEnergy;
    energyFrac = nEnergy / pnGamma->getEnergy();

    if (eventType == 1) {
      histograms_.fill("1n_ke:2nd_h_ke", nEnergy, subleading_ke);
      histograms_.fill("1n_neutron_energy", nEnergy);
      histograms_.fill("1n_energy_diff", energyDiff);
      histograms_.fill("1n_energy_frac", energyFrac);
    } else if (eventType == 2) {
      histograms_.fill("2n_n2_energy", subleading_ke);
      auto energyFrac2n = (nEnergy + subleading_ke) / pnGamma->getEnergy();
      histograms_.fill("2n_energy_frac", energyFrac2n);
      histograms_.fill("2n_energy_other", pnGamma->getEnergy() - energyFrac2n);

    } else if (eventType == 17) {
      histograms_.fill("1kp_ke:2nd_h_ke", nEnergy, subleading_ke);
      histograms_.fill("1kp_energy", nEnergy);
      histograms_.fill("1kp_energy_diff", energyDiff);
      histograms_.fill("1kp_energy_frac", energyFrac);
    } else if (eventType == 16 || eventType == 18) {
      histograms_.fill("1k0_ke:2nd_h_ke", nEnergy, subleading_ke);
      histograms_.fill("1k0_energy", nEnergy);
      histograms_.fill("1k0_energy_diff", energyDiff);
      histograms_.fill("1k0_energy_frac", energyFrac);
    }

    auto nPdgID{abs(pnDaughters[0]->getPdgID())};
    auto nEventType{-10};
    if (nPdgID == 2112) {
      nEventType = 1;
    } else if (nPdgID == 2212) {
      nEventType = 2;
    } else if (nPdgID == 211) {
      nEventType = 3;
    } else if (nPdgID == 111) {
      nEventType = 4;
    }

    histograms_.fill("1n_event_type", nEventType);
  }
}

int PhotoNuclearDQM::classifyEvent(
    const std::vector<const ldmx::SimParticle *> daughters, double threshold) {
  short n{0}, p{0}, pi{0}, pi0{0}, exotic{0}, k0l{0}, kp{0}, k0s{0}, lambda{0};

  // Loop through all of the PN daughters and extract kinematic
  // information.
  for (const auto &daughter : daughters) {
    // Calculate the kinetic energy
    auto ke{daughter->getEnergy() - daughter->getMass()};

    // If the kinetic energy is below threshold, continue
    if (ke <= threshold)
      continue;

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
    return nothing_hard;
  }
  if (count == 1) {
    if (n == 1) {
      return single_neutron;
    } else if (p == 1) {
      return single_proton;
    } else if (pi0 == 1) {
      return single_neutral_pion;
    } else if (pi == 1) {
      return single_charged_pion;
    }
  }
  if (count == 2) {
    if (n == 2) {
      return two_neutrons;
    } else if (n == 1 && p == 1) {
      return proton_neutron;
    } else if (p == 2) {
      return two_protons;
    } else if (pi == 2) {
      return two_charged_pions;
    } else if (pi == 1 && nucleons == 1) {
      return single_charged_pion_and_nucleon;
    } else if (pi0 == 1 && nucleons == 1) {
      return single_neutral_pion_and_nucleon;
    }
  }

  if (count == 3) {
    if (pi == 1 && nucleons == 2) {
      return single_charged_pion_and_two_nucleons;
    } else if (pi == 2 && nucleons == 1) {
      return two_charged_pions_and_nucleon;
    } // else
    else if (pi0 == 1 && nucleons == 2) {
      return single_neutral_pion_and_two_nucleons;
    } else if (pi0 == 1 && nucleons == 1 && pi == 1) {
      return single_neutral_pion_charged_pion_and_nucleon;
    }
  }
  if (count >= 3 && count == n) {
    return three_or_more_neutrons;
  }

  if (kaons >= 1) {
    if (k0l == 1) {
      return klong;
    } else if (kp == 1) {
      return charged_kaon;
    } else if (k0s == 1) {
      return kshort;
    }
  }
  if (exotics == count && count != 0) {
    return exotics;
  }

  // TODO Remove, broken
  if (pi0 == 1) {
    if ((pi == 1) && ((p == 1) || (n == 1)) && (kaons == 0 && exotic == 0)) {
      return 12;
    }
  }

  return multibody;
}

int PhotoNuclearDQM::classifyCompactEvent(
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
      if (pdgID == 2112)
        n++;
      else if (pdgID == 130)
        k0l++;
      else if (pdgID == 321)
        kp++;
      else if (pdgID == 310)
        k0s++;
      continue;
    }

    if ((pdgID == 2112) && ke > threshold)
      n_t++;
  }

  int neutral_kaons{k0l + k0s};

  if (n != 0)
    return 0;
  if (kp != 0)
    return 1;
  if (neutral_kaons != 0)
    return 2;
  if (n_t == 2)
    return 3;
  if (soft == daughters.size())
    return 4;

  return 5;
}

void PhotoNuclearDQM::printParticleTree(
    std::map<int, ldmx::SimParticle> particleMap) {
  std::vector<int> printedParticles;

  // Loop through the particle map
  for (auto const &[trackID, simParticle] : particleMap) {
    // Print the particle only if it has daughters
    if ((simParticle.getDaughters().size() != 0) &
        (std::find(printedParticles.begin(), printedParticles.end(), trackID) ==
         printedParticles.end())) {
      simParticle.Print();
      printedParticles.push_back(trackID);

      // Print the daughters
      std::vector<int> printedDaughters =
          printDaughters(particleMap, simParticle, 1);
      printedParticles.insert(printedParticles.end(), printedDaughters.begin(),
                              printedDaughters.end());
    }
  }
}

std::vector<int>
PhotoNuclearDQM::printDaughters(std::map<int, ldmx::SimParticle> particleMap,
                                const ldmx::SimParticle particle, int depth) {
  std::vector<int> printedParticles;

  // Don't print anything if a particle doesn't have any daughters
  if (particle.getDaughters().size() == 0)
    return printedParticles;

  // Generate the prefix
  std::string prefix{""};
  for (auto i{0}; i < depth; ++i)
    prefix += "\t";

  // Loop through all of the daughter particles and print them
  for (const auto &daughter : particle.getDaughters()) {
    // Print the ith daughter particle
    std::cout << prefix;
    particleMap[daughter].Print();
    printedParticles.push_back(daughter);

    // Print the Daughters
    std::vector<int> printedDaughters =
        printDaughters(particleMap, particleMap[daughter], depth + 1);
    printedParticles.insert(printedParticles.end(), printedDaughters.begin(),
                            printedDaughters.end());
  }
}

} // namespace dqm

DECLARE_ANALYZER_NS(dqm, PhotoNuclearDQM)
