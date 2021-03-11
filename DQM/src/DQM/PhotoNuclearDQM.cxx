
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

PhotoNuclearDQM::PhotoNuclearDQM(const std::string& name,
                                 framework::Process& process)
    : framework::Analyzer(name, process) {}

PhotoNuclearDQM::~PhotoNuclearDQM() {}

void PhotoNuclearDQM::onProcessStart() {
  std::vector<std::string> labels = {"",
                                     "Nothing hard",   // 0
                                     "1 n",            // 1
                                     "2 n",            // 2
                                     "#geq 3 n",       // 3
                                     "1 #pi",          // 4
                                     "2 #pi",          // 5
                                     "1 #pi_{0}",      // 6
                                     "1 #pi A",        // 7
                                     "1 #pi 2 A",      // 8
                                     "2 #pi A",        // 9
                                     "1 #pi_{0} A",    // 10
                                     "1 #pi_{0} 2 A",  // 11
                                     "#pi_{0} #pi A",  // 12
                                     "1 p",            // 13
                                     "2 p",            // 14
                                     "pn",             // 15
                                     "K^{0}_{L} X",    // 16
                                     "K X",            // 17
                                     "K^{0}_{S} X",    // 18
                                     "exotics",        // 19
                                     "multi-body",     // 20
                                     ""};

  std::vector<TH1*> hists = {
      histograms_.get("event_type"),
      histograms_.get("event_type_500mev"),
      histograms_.get("event_type_2000mev"),

  };

  for (int ilabel{1}; ilabel < labels.size(); ++ilabel) {
    for (auto& hist : hists) {
      hist->GetXaxis()->SetBinLabel(ilabel, labels[ilabel - 1].c_str());
    }
  }

  labels = {"",
            "1 n",      // 0
            "K#pm X",   // 1
            "1 K^{0}",  // 2
            "2 n",      // 3
            "Soft",     // 4
            "Other",    // 5
            ""};

  hists = {
      histograms_.get("event_type_compact"),
      histograms_.get("event_type_compact_500mev"),
      histograms_.get("event_type_compact_2000mev"),
  };

  for (int ilabel{1}; ilabel < labels.size(); ++ilabel) {
    for (auto& hist : hists) {
      hist->GetXaxis()->SetBinLabel(ilabel, labels[ilabel - 1].c_str());
    }
  }

  std::vector<std::string> n_labels = {"",         "",
                                       "nn",        // 1
                                       "pn",        // 2
                                       "#pi^{+}n",  // 3
                                       "#pi^{0}n",  // 4
                                       ""};

  TH1* hist = histograms_.get("1n_event_type");
  for (int ilabel{1}; ilabel < n_labels.size(); ++ilabel) {
    hist->GetXaxis()->SetBinLabel(ilabel, n_labels[ilabel - 1].c_str());
  }
}

void PhotoNuclearDQM::configure(framework::config::Parameters& parameters) {}

void PhotoNuclearDQM::analyze(const framework::Event& event) {
  // Get the particle map from the event.  If the particle map is empty,
  // don't process the event.
  auto particleMap{event.getMap<int, ldmx::SimParticle>("SimParticles")};
  if (particleMap.size() == 0) return;

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
    std::cout << "[ PhotoNuclearDQM ]: PN Daughter is lost, skipping."
              << std::endl;
    return;
  }

  histograms_.fill("pn_particle_mult", pnGamma->getDaughters().size());
  histograms_.fill("pn_gamma_energy", pnGamma->getEnergy());
  histograms_.fill("pn_gamma_int_z", pnGamma->getEndPoint()[2]);
  histograms_.fill("pn_gamma_vertex_x", pnGamma->getVertex()[0]);
  histograms_.fill("pn_gamma_vertex_y", pnGamma->getVertex()[1]);
  histograms_.fill("pn_gamma_vertex_z", pnGamma->getVertex()[2]);

  double lke{-1}, lt{-1};
  double lpke{-1}, lpt{-1};
  double lnke{-1}, lnt{-1};
  double lpike{-1}, lpit{-1};

  std::vector<const ldmx::SimParticle*> pnDaughters;

  // Loop through all of the PN daughters and extract kinematic
  // information.
  for (const auto& daughterTrackID : pnGamma->getDaughters()) {
    // skip daughters that weren't saved
    if (particleMap.count(daughterTrackID) == 0) continue;

    auto daughter{&(particleMap.at(daughterTrackID))};

    // Get the PDG ID
    auto pdgID{daughter->getPdgID()};

    // Ignore photons and nuclei
    if (pdgID == 22 || pdgID > 10000) continue;

    // Calculate the kinetic energy
    double ke{daughter->getEnergy() - daughter->getMass()};

    std::vector<double> vec{daughter->getMomentum()};
    TVector3 pvec(vec[0], vec[1], vec[2]);

    //  Calculate the polar angle
    auto theta{pvec.Theta() * (180 / 3.14159)};

    if (lke < ke) {
      lke = ke;
      lt = theta;
    }

    if ((pdgID == 2112) && (lnke < ke)) {
      lnke = ke;
      lnt = theta;
    }

    if ((pdgID == 2212) && (lpke < ke)) {
      lpke = ke;
      lpt = theta;
    }

    if (((abs(pdgID) == 211) || (pdgID == 111)) && (lpike < ke)) {
      lpike = ke;
      lpit = theta;
    }

    pnDaughters.push_back(daughter);
  }

  histograms_.fill("hardest_ke", lke);
  histograms_.fill("hardest_theta", lt);
  histograms_.fill("h_ke_h_theta", lke, lt);
  histograms_.fill("hardest_p_ke", lpke);
  histograms_.fill("hardest_p_theta", lpt);
  histograms_.fill("hardest_n_ke", lnke);
  histograms_.fill("hardest_n_theta", lnt);
  histograms_.fill("hardest_pi_ke", lpike);
  histograms_.fill("hardest_pi_theta", lpit);

  // Classify the event
  auto eventType{classifyEvent(pnDaughters, 200)};
  auto eventType500MeV{classifyEvent(pnDaughters, 500)};
  auto eventType2000MeV{classifyEvent(pnDaughters, 2000)};

  auto eventTypeComp{classifyCompactEvent(pnGamma, pnDaughters, 200)};
  auto eventTypeComp500MeV{classifyCompactEvent(pnGamma, pnDaughters, 200)};
  auto eventTypeComp2000MeV{classifyCompactEvent(pnGamma, pnDaughters, 200)};

  histograms_.fill("event_type", eventType);
  histograms_.fill("event_type_500mev", eventType500MeV);
  histograms_.fill("event_type_2000mev", eventType2000MeV);

  histograms_.fill("event_type_compact", eventTypeComp);
  histograms_.fill("event_type_compact_500mev", eventTypeComp500MeV);
  histograms_.fill("event_type_compact_2000mev", eventTypeComp2000MeV);

  double slke{-9999};
  double nEnergy{-9999}, energyDiff{-9999}, energyFrac{-9999};

  if (eventType == 1 || eventType == 17 || eventType == 16 || eventType == 18 ||
      eventType == 2) {
    std::sort(pnDaughters.begin(), pnDaughters.end(),
              [](const auto& lhs, const auto& rhs) {
                double lhs_ke = lhs->getEnergy() - lhs->getMass();
                double rhs_ke = rhs->getEnergy() - rhs->getMass();
                return lhs_ke > rhs_ke;
              });

    nEnergy = pnDaughters[0]->getEnergy() - pnDaughters[0]->getMass();
    slke = -9999;
    if (pnDaughters.size() > 1)
      slke = pnDaughters[1]->getEnergy() - pnDaughters[1]->getMass();
    energyDiff = pnGamma->getEnergy() - nEnergy;
    energyFrac = nEnergy / pnGamma->getEnergy();

    if (eventType == 1) {
      histograms_.fill("1n_ke:2nd_h_ke", nEnergy, slke);
      histograms_.fill("1n_neutron_energy", nEnergy);
      histograms_.fill("1n_energy_diff", energyDiff);
      histograms_.fill("1n_energy_frac", energyFrac);
    } else if (eventType == 2) {
      histograms_.fill("2n_n2_energy", slke);
      auto energyFrac2n = (nEnergy + slke) / pnGamma->getEnergy();
      histograms_.fill("2n_energy_frac", energyFrac2n);
      histograms_.fill("2n_energy_other", pnGamma->getEnergy() - energyFrac2n);

    } else if (eventType == 17) {
      histograms_.fill("1kp_ke:2nd_h_ke", nEnergy, slke);
      histograms_.fill("1kp_energy", nEnergy);
      histograms_.fill("1kp_energy_diff", energyDiff);
      histograms_.fill("1kp_energy_frac", energyFrac);
    } else if (eventType == 16 || eventType == 18) {
      histograms_.fill("1k0_ke:2nd_h_ke", nEnergy, slke);
      histograms_.fill("1k0_energy", nEnergy);
      histograms_.fill("1k0_energy_diff", energyDiff);
      histograms_.fill("1k0_energy_frac", energyFrac);
    }

    auto nPdgID{abs(pnDaughters[0]->getPdgID())};
    auto nEventType{-10};
    if (nPdgID == 2112)
      nEventType = 1;
    else if (nPdgID == 2212)
      nEventType = 2;
    else if (nPdgID == 211)
      nEventType = 3;
    else if (nPdgID == 111)
      nEventType = 4;

    histograms_.fill("1n_event_type", nEventType);
  }
}

int PhotoNuclearDQM::classifyEvent(
    const std::vector<const ldmx::SimParticle*> daughters, double threshold) {
  short n{0}, p{0}, pi{0}, pi0{0}, exotic{0}, k0l{0}, kp{0}, k0s{0}, lambda{0};

  // Loop through all of the PN daughters and extract kinematic
  // information.
  for (const auto& daughter : daughters) {
    // Calculate the kinetic energy
    auto ke{daughter->getEnergy() - daughter->getMass()};

    // If the kinetic energy is below threshold, continue
    if (ke <= threshold) continue;

    // Get the PDG ID
    auto pdgID{abs(daughter->getPdgID())};

    if (pdgID == 2112)
      n++;
    else if (pdgID == 2212)
      p++;
    else if (pdgID == 211)
      pi++;
    else if (pdgID == 111)
      pi0++;
    else if (pdgID == 130)
      k0l++;
    else if (pdgID == 321)
      kp++;
    else if (pdgID == 310)
      k0s++;
    else
      exotic++;
  }

  int kaons = k0l + kp + k0s;
  int nucleons = n + p;
  int pions = pi + pi0;
  int count = nucleons + pions + exotic + kaons;
  int count_a = p + pions + exotic + kaons;
  int count_b = pions + exotic + kaons;
  int count_c = nucleons + pi0 + exotic + kaons;
  int count_d = n + pi0 + exotic + k0l + kp + k0s;
  int count_e = p + pi0 + exotic + k0l + kp + k0s;
  int count_f = n + p + pi + exotic + k0l + kp + k0s;
  int count_g = n + pi + pi0 + exotic + k0l + kp + k0s;
  int count_h = n + p + pi + pi0 + k0l + kp + k0s;
  int count_i = p + pi + exotic + k0l + kp + k0s;
  int count_j = n + pi + exotic + k0l + kp + k0s;
  int count_k = nucleons + pions + exotic + kp + k0s;
  int count_l = nucleons + pions + exotic + k0l + k0s;
  int count_m = nucleons + pions + exotic + kp + k0l;
  int count_n = pi0 + exotic + kaons;
  int count_o = pi + exotic + kaons;
  int count_p = exotic + kaons;

  if (count == 0) return 0;  // Nothing hard

  if (n == 1) {
    if (count_a == 0)
      return 1;  // 1n
    else if ((p == 1) && (count_b == 0))
      return 15;  // pn
  }

  if ((n == 2) && (count_a == 0)) return 2;  // 2 n

  if ((n >= 3) && (count_a == 0)) return 3;  // >= 3 n

  if (pi == 1) {
    if (count_c == 0)
      return 4;  // 1 pi
    else if ((p == 1) && (count_d == 0))
      return 7;  // 1 pi 1 p
    else if ((p == 2) && (count_d == 0))
      return 8;  // 1 pi 1 p
    else if ((n == 1) && (count_e == 0))
      return 7;  // 1 pi 1 n
    else if ((n == 2) && (count_e == 0))
      return 8;  // 1 pi 1 n
    else if ((n == 1) && (p == 1) && (count_n == 0))
      return 8;
  }

  if (pi == 2) {
    if (count_c == 0)
      return 5;  // 2pi
    else if ((p == 1) && (count_d == 0))
      return 9;  // 2pi p
    else if ((n == 1) && (count_e == 0))
      return 9;  // 2pi n
  }

  if (pi0 == 1) {
    if (count_f == 0)
      return 6;  // 1 pi0
    else if ((n == 1) && (count_i == 0))
      return 10;  // 1pi0 1 p
    else if ((n == 2) && (count_i == 0))
      return 11;  // 1pi0 1 p
    else if ((p == 1) && (count_j == 0))
      return 10;  // 1pi0 1 n
    else if ((p == 2) && (count_j == 0))
      return 11;
    else if ((n == 1) && (p == 1) && (count_o == 0))
      return 11;
    else if ((pi == 1) && ((p == 1) || (n == 1)) && (count_p == 0))
      return 12;
  }

  if ((p == 1) && (count_g == 0)) return 13;  // 1 p
  if ((p == 2) && (count_g == 0)) return 14;  // 2 p

  if (k0l == 1) return 16;
  if (kp == 1) return 17;
  if (k0s == 1) return 18;

  if ((exotic > 0) && (count_h == 0)) return 19;

  return 20;
}

int PhotoNuclearDQM::classifyCompactEvent(
    const ldmx::SimParticle* pnGamma,
    const std::vector<const ldmx::SimParticle*> daughters, double threshold) {
  short n{0}, n_t{0}, k0l{0}, kp{0}, k0s{0}, soft{0};

  // Loop through all of the PN daughters and extract kinematic
  // information.
  for (const auto& daughter : daughters) {
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

    if ((pdgID == 2112) && ke > threshold) n_t++;
  }

  int neutral_kaons{k0l + k0s};

  if (n != 0) return 0;
  if (kp != 0) return 1;
  if (neutral_kaons != 0) return 2;
  if (n_t == 2) return 3;
  if (soft == daughters.size()) return 4;

  return 5;
}

void PhotoNuclearDQM::printParticleTree(
    std::map<int, ldmx::SimParticle> particleMap) {
  std::vector<int> printedParticles;

  // Loop through the particle map
  for (auto const& [trackID, simParticle] : particleMap) {
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

std::vector<int> PhotoNuclearDQM::printDaughters(
    std::map<int, ldmx::SimParticle> particleMap,
    const ldmx::SimParticle particle, int depth) {
  std::vector<int> printedParticles;

  // Don't print anything if a particle doesn't have any daughters
  if (particle.getDaughters().size() == 0) return printedParticles;

  // Generate the prefix
  std::string prefix{""};
  for (auto i{0}; i < depth; ++i) prefix += "\t";

  // Loop through all of the daughter particles and print them
  for (const auto& daughter : particle.getDaughters()) {
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

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, PhotoNuclearDQM)
