#include "Ecal/EcalClusterAnalyzer.h"

#include "Ecal/Event/EcalHit.h"
#include "Ecal/Event/EcalCluster.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "DetDescr/SimSpecialID.h"

#include <algorithm>
#include <fstream>
#include <iostream>

namespace ecal {

void EcalClusterAnalyzer::configure(framework::config::Parameters& ps) {
  ecalSimHitColl_ = ps.getParameter<std::string>("ecalSimHitColl");
  ecalSimHitPass_ = ps.getParameter<std::string>("ecalSimHitPass");
  
  recHitCollName_ = ps.getParameter<std::string>("recHitCollName");
  recHitPassName_ = ps.getParameter<std::string>("recHitPassName");

  clusterCollName_ = ps.getParameter<std::string>("clusterCollName");
  clusterPassName_ = ps.getParameter<std::string>("clusterPassName");
  return;
}

void EcalClusterAnalyzer::analyze(const framework::Event& event) {
  auto ecalRecHits{event.getCollection<ldmx::EcalHit>(recHitCollName_, recHitPassName_)};
  auto ecalSimHits{event.getCollection<ldmx::SimCalorimeterHit>(ecalSimHitColl_, ecalSimHitPass_)};
  auto ecalClusters{event.getCollection<ldmx::EcalCluster>(clusterCollName_, clusterPassName_)};

  std::unordered_map<int, std::tuple<int, double, double>> hitInfo;
  hitInfo.reserve(ecalRecHits.size());

  std::vector<float> pos1;
  std::vector<float> pos2;
  bool p1 = false;
  bool p2 = false;
  double dist;
  auto ecalSpHits{event.getCollection<ldmx::SimTrackerHit>("EcalScoringPlaneHits")};
  for (ldmx::SimTrackerHit &spHit : ecalSpHits) {
    if (spHit.getTrackID() == 1) {
      pos1 = spHit.getPosition();
      p1 = true;
    } else if (spHit.getTrackID() == 2) {
      pos2 = spHit.getPosition();
      p2 = true;
    }
  }
  if (p1 && p2) dist = std::sqrt(std::pow((pos1[0]-pos2[0]), 2) + std::pow((pos1[1]-pos2[1]), 2));

  for (const auto& hit : ecalRecHits) {
    auto it = std::find_if(ecalSimHits.begin(), ecalSimHits.end(), [&hit](const auto& simHit) { return simHit.getID() == hit.getID(); });
    if (it != ecalSimHits.end()) {
      int ancestor = 0;
      int prevAncestor = 0;
      bool tagged = false;
      int tag = 0;
      double edep1 = 0;
      double edep2 = 0;
      // double elost = 0;
      for (int i = 0; i < it->getNumberOfContribs(); i++) {
        const auto& c = it->getContrib(i);
        // if (!tagged && c.originID != 1 && c.originID != 2) {
        //   // should not happen anymore
        //   tag = 0;
        //   tagged = true;
        // } else 
        ancestor = c.originID;
        if (ancestor == 1) edep1 += c.edep;
        else if (ancestor == 2) edep2 += c.edep;
        // else elost += c.edep;
        if (!tagged && i != 0 && prevAncestor != ancestor) {
          // mixed case
          tag = 3;
          tagged = true;
        }
        prevAncestor = ancestor;
      }
      if (!tagged) {
        tag = prevAncestor;        
      }
      histograms_.fill("ancestors", tag);
      hitInfo.insert({hit.getID(), {tag, edep1, edep2}});
    }
  }

  int clusteredHits = 0;

  for (const auto& cl : ecalClusters) {
    // double unclear = 0;
    double n1 = 0;
    double n2 = 0;
    double m = 0;
    double e1 = 0;
    double e2 = 0;
    double em = 0;
    // double elost = 0;
    const auto& hitIDs = cl.getHitIDs();
    const auto& mixedHitIDs = cl.getMixedHitIDs();
    for (const auto& id : hitIDs) {
      auto it = hitInfo.find(id);
      if (it != hitInfo.end()) {
        auto t = it->second;
        switch(std::get<0>(t)) {
          // case 0:
          //   unclear++;
          //   break;
          case 1:
            n1++;
            break;
          case 2:
            n2++;
            break;
          case 3:
            m++;
            em += std::get<1>(t) + std::get<2>(t);
            break;
          default:
            break;
        }
        
        e1 += std::get<1>(t);
        e2 += std::get<2>(t);
        // elost += std::get<3>(t);
      }
      clusteredHits++;
    }
    for (const auto &[id, percentage] : mixedHitIDs) {
      auto it = hitInfo.find(id);
      if (it != hitInfo.end()) {
        auto it = hitInfo.find(id);
        if (it != hitInfo.end()) {
          auto t = it->second;
          switch(std::get<0>(t)) {
            // case 0:
            //   unclear++;
            //   break;
            case 1:
              n1++;
              break;
            case 2:
              n2++;
              break;
            case 3:
              m++;
              em += (std::get<1>(t) + std::get<2>(t))*percentage;
              break;
            default:
              break;
          }
          
          e1 += std::get<1>(t)*percentage;
          e2 += std::get<2>(t)*percentage;
          // elost += std::get<3>(t);
        }
      }
    }
    if ((e1 + e2) > 0) {
      double eperc;
      // double eunfiltered;
      double emixed;
      if (e1 >= e2) {
        eperc = 100.*(e1/(e1+e2));
        // eunfiltered = 100.*(e1/(e1+e2));
      } else {
        eperc = 100.*(e2/(e1+e2));
        // eunfiltered = 100.*(e2/(e1+e2));
      }

      histograms_.fill("energy_percentage", eperc);
      if (em > 0) {
        histograms_.fill("mixed_hit_energy", 100.*(m/(e1+e2)));
      }

      histograms_.fill("total_energy_vs_hits", e1+e2, cl.getHitIDs().size());
      histograms_.fill("total_energy_vs_purity", (e1+e2), eperc);

      histograms_.fill("distance_energy_purity", dist, eperc);
    }
    if ((n1 + n2) > 0) {
      double percentage;
      // double unfiltered;
      if (n1 >= n2) {
        percentage = 100.*(n1/(n1+n2));
        // unfiltered = 100.*(n1/(n1+n2+m+unclear));
      } else {
        percentage = 100.*(n2/(n1+n2));
        // unfiltered = 100.*(n2/(n1+n2+m+unclear));
      }
      histograms_.fill("same_ancestor", percentage);
      histograms_.fill("distance_purity", dist, percentage);
    }
    if ((n1 + n2 + m) > 0) {
      histograms_.fill("mixed_ancestry", 100.*(m/(n1+n2+m)));
    }
  }

  histograms_.fill("clusterless_hits", (ecalRecHits.size()-clusteredHits));

  
}
}
DECLARE_ANALYZER_NS(ecal, EcalClusterAnalyzer)
