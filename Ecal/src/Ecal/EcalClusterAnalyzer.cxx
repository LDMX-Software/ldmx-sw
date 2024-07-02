#include "Ecal/EcalClusterAnalyzer.h"

#include "Ecal/Event/EcalHit.h"
#include "Ecal/Event/EcalCluster.h"
#include "SimCore/Event/SimCalorimeterHit.h"
// #include "SimCore/G4User/TrackingAction.h"

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

  // depth_ = ps.getParameter<int>("depth");
  return;
}

void EcalClusterAnalyzer::analyze(const framework::Event& event) {
  auto ecalRecHits{event.getCollection<ldmx::EcalHit>(recHitCollName_, recHitPassName_)};
  auto ecalSimHits{event.getCollection<ldmx::SimCalorimeterHit>(ecalSimHitColl_, ecalSimHitPass_)};
  auto ecalClusters{event.getCollection<ldmx::EcalCluster>(clusterCollName_, clusterPassName_)};

  std::unordered_map<int, int> ancestorTag;
  ancestorTag.reserve(ecalRecHits.size());

  // auto trackMap{simcore::g4user::TrackingAction::get()->getTrackMap()};

  for (const auto& hit : ecalRecHits) {
    auto it = std::find_if(ecalSimHits.begin(), ecalSimHits.end(), [&hit](const auto& simHit) { return simHit.getID() == hit.getID(); });
    if (it != ecalSimHits.end()) {
      int ancestor = 0;
      int prevAncestor = 0;
      bool tagged = false;
      int tag = 0;
      for (int i = 0; i < it->getNumberOfContribs(); i++) {
        const auto& c = it->getContrib(i);
        if (c.originID != 1 && c.originID != 2) {
          // trace back to 1/2
          // bool dec1 = trackMap.isDescendant(c.incidentID, 1, depth_);
          // bool dec2 = trackMap.isDescendant(c.incidentID, 2, depth_);
          // if (dec1 && dec2) {
          //   ancestor = 3;
          // } else if (dec1) {
          //   ancestor = 1;
          // } else if (dec2) {
          //   ancestor = 2;
          // } else {
          //   search was not deep enough -- while loop, update depth? Or just make depth huge?
          // }
          tag = 0;
          tagged = true;
          break;
        } else ancestor = c.originID;
        if (i != 0 && prevAncestor != ancestor) {
          // mixed case
          tag = 3;
          tagged = true;
          break;
        }
        prevAncestor = ancestor;
      }
      if (!tagged) {
        tag = prevAncestor;        
      }
      histograms_.fill("ancestors", tag);
      ancestorTag.insert({hit.getID(), tag});
    }
  }

  int clusteredHits = 0;

  for (const auto& cl : ecalClusters) {
    double unclear = 0;
    double n1 = 0;
    double n2 = 0;
    double m = 0;
    const auto& hitIDs = cl.getHitIDs();
    for (const auto& id : hitIDs) {
      auto it = ancestorTag.find(id);
      if (it != ancestorTag.end()) {
        switch(it->second) {
          case 0:
            unclear++;
            break;
          case 1:
            n1++;
            break;
          case 2:
            n2++;
            break;
          case 3:
            m++;
            break;
          default:
            break;
        }
      }
      clusteredHits++;
    }
    if ((n1 + n2) > 0) {
      double percentage;
      double unfiltered;
      if (n1 >= n2) {
        percentage = 100.*(n1/(n1+n2));
        unfiltered = 100.*(n1/(n1+n2+m+unclear));
      } else {
        percentage = 100.*(n2/(n1+n2));
        unfiltered = 100.*(n2/(n1+n2+m+unclear));
      }
      histograms_.fill("same_ancestor", percentage);
      histograms_.fill("UF_same_ancestor", unfiltered);
    }
    if ((n1 + n2 + m) > 0) {
      histograms_.fill("mixed_ancestry", 100.*(m/(n1+n2+m)));
      histograms_.fill("UF_mixed_ancestry", 100.*(m/(n1+n2+m+unclear)));
    }
    if ((n1 + n2 + m + unclear) > 0) {
      histograms_.fill("unclear_ancestry", 100.*(unclear/(n1+n2+m+unclear)));
    }
  }

  histograms_.fill("clusterless_hits", (ecalRecHits.size()-clusteredHits));
}

}
DECLARE_ANALYZER_NS(ecal, EcalClusterAnalyzer)
