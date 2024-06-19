#include "DQM/VisGenerator.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "Ecal/Event/EcalHit.h"
#include "Ecal/Event/EcalCluster.h"
// #include "DQM/json.hpp"

#include <fstream>
#include <iostream>
using json = nlohmann::json;

namespace dqm {

  void VisGenerator::configure(framework::config::Parameters& ps) {
    ecalSimHitColl_ = ps.getParameter<std::string>("ecalSimHitColl");
    ecalSimHitPass_ = ps.getParameter<std::string>("ecalSimHitPass");
    ecalRecHitColl_ = ps.getParameter<std::string>("ecalRecHitColl");
    ecalRecHitPass_ = ps.getParameter<std::string>("ecalRecHitPass");
    ecalClusterColl_ = ps.getParameter<std::string>("ecalClusterColl");
    ecalClusterPass_ = ps.getParameter<std::string>("ecalClusterPass");
    filename_ = ps.getParameter<std::string>("filename");
    runNbr_ = ps.getParameter<int>("runNumber");
    return;
  }

  void VisGenerator::analyze(const framework::Event& event) {
    int colorIt = 0; // keeping this in case we need it for other objects    

    // ----- EVENT HEADER -----
    std::string eKey = "EVENT_KEY_" + std::to_string(event.getEventNumber());
    j[eKey]["event number"] = event.getEventNumber();
    j[eKey]["run number"] = runNbr_;

    // ----- HITS -----
    j[eKey]["Hits"] = json::object();

    // ----- CLUSTER-RECHIT CONNECTION -----
    std::vector<ldmx::EcalCluster> ecalClusters = event.getCollection<ldmx::EcalCluster>(ecalClusterColl_, ecalClusterPass_);
    std::vector<ldmx::EcalHit> ecalRecHits = event.getCollection<ldmx::EcalHit>(ecalRecHitColl_, ecalRecHitPass_);

    std::unordered_map<int, int> hitToCluster;
    hitToCluster.reserve(ecalRecHits.size());

    int clusterID = 1;
    double clusterSize = 15.0; // scale to energy?
    double singleClusterSize = 2.0;
    float clusterHitSize = 5.0;
    float clusterlessHitSize = 2.0;

    j[eKey]["Hits"]["single_clusters"] = json::array();

    for (auto const& cl : ecalClusters) {
      json cluster = json::object();
      cluster["type"] = "Box";
      if (cl.getHitIDs().size() != 0) { // if cluster is larger than just one hit
        // create collection for cluster
        std::string& hex = colors[colorIt % colors.size()];
        std::string cKey = "cluster_" + std::to_string(clusterID);
        j[eKey]["Hits"][cKey] = json::array();
        // create centroid object
        cluster["color"] = hex;
        cluster["pos"] = { cl.getCentroidX(), cl.getCentroidY(), cl.getCentroidZ(),
                            clusterSize, clusterSize, clusterSize };
        j[eKey]["Hits"][cKey].push_back(cluster);
        for (auto const& clHitID : cl.getHitIDs()) {
          // map hit id to cluster it belongs to
          hitToCluster.insert({clHitID, clusterID});
        }
        clusterID++;
        colorIt++;
      } else {
        cluster["color"] = "0xFF0000";
        cluster["pos"] = { cl.getCentroidX(), cl.getCentroidY(), cl.getCentroidZ(),
                          singleClusterSize, singleClusterSize, singleClusterSize };
        j[eKey]["Hits"]["single_clusters"].push_back(cluster);
      }
    }

    j[eKey]["Hits"]["clusterless_hits"] = json::array();

    for (auto const& hit : ecalRecHits) {
      if (!hit.isNoise()) {
        auto& id = hitToCluster[hit.getID()];
        json h = json::object();
        h["type"] = "Box";
        if (id != 0) { // if hit is associated to a cluster
          // add hit to cluster collection
          // color will automatically be set to same as centroid
          std::string cKey = "cluster_" + std::to_string(id);
          h["pos"] = { hit.getXPos(), hit.getYPos(), hit.getZPos(),
                          clusterHitSize, clusterHitSize, clusterHitSize };
          j[eKey]["Hits"][cKey].push_back(h);
        } else {
          h["color"] = "0x000000";
          h["pos"] = { hit.getXPos(), hit.getYPos(), hit.getZPos(),
                          clusterlessHitSize, clusterlessHitSize, clusterlessHitSize };
          j[eKey]["Hits"]["clusterless_hits"].push_back(h);
        }
      }
    }

    // // ----- TRACKS -----
    j[eKey]["Tracks"] = json::object();

    // GROUND TRUTH (SIMULATED) PATHS
    j[eKey]["Tracks"]["ground_truth_tracks"] = json::array();
    auto particle_map{event.getMap<int, ldmx::SimParticle>("SimParticles")};
    for (const auto& it : particle_map) {
      json track = json::object();
      const auto& start = it.second.getVertex();
      const auto& end = it.second.getEndPoint();
      track["pos"] = { { start[0], start[1], start[2] },
                        { end[0], end[1], end[2] } };
      j[eKey]["Tracks"]["ground_truth_tracks"].push_back(track);
    }
    return;
  }

void VisGenerator::onProcessEnd(){
    std::ofstream file(filename_);
    file << std::setw(2) << j << std::endl;
    file.close();
    return;
};

}

DECLARE_ANALYZER_NS(dqm, VisGenerator)
