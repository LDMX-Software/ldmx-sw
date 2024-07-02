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
    includeGroundTruth_ = ps.getParameter<bool>("includeGroundTruth");
    ecalSimHitColl_ = ps.getParameter<std::string>("ecalSimHitColl");
    ecalSimHitPass_ = ps.getParameter<std::string>("ecalSimHitPass");

    includeEcalRecHits_ = ps.getParameter<bool>("includeEcalRecHits");
    ecalRecHitColl_ = ps.getParameter<std::string>("ecalRecHitColl");
    ecalRecHitPass_ = ps.getParameter<std::string>("ecalRecHitPass");

    includeEcalClusters_ = ps.getParameter<bool>("includeEcalClusters");
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

    // GROUND TRUTH HIT INFO
    auto ecalSimHits{event.getCollection<ldmx::SimCalorimeterHit>(ecalSimHitColl_, ecalSimHitPass_)};

    // CLUSTER-RECHIT CONNECTION
    if (includeEcalClusters_ || includeEcalRecHits_) {
      std::vector<ldmx::EcalCluster> ecalClusters;
      std::vector<ldmx::EcalHit> ecalRecHits;
      std::unordered_map<int, int> hitToCluster;
      if (includeEcalClusters_) ecalClusters = event.getCollection<ldmx::EcalCluster>(ecalClusterColl_, ecalClusterPass_);
      if (includeEcalRecHits_) {
        ecalRecHits = event.getCollection<ldmx::EcalHit>(ecalRecHitColl_, ecalRecHitPass_);
        if (includeEcalClusters_) hitToCluster.reserve(ecalRecHits.size());
      }

      int clusterID = 1;
      double clusterSize = 15.0; // scale to energy?
      double singleClusterSize = 1.0;
      float clusterHitSize = 2.0;
      float clusterlessHitSize = 1.0;

      int singleHitClusters = 0;

      if (includeEcalClusters_) {
        // Clusters with zero hits should be removed, this is for debug
        j[eKey]["Hits"]["empty_clusters"] = json::array();

        for (auto const& cl : ecalClusters) {
          json cluster = json::object();
          cluster["type"] = "Box";
          if (cl.getHitIDs().size() != 0) { // if cluster is larger than just one hit
            // create collection for cluster
            std::string& hex = colors[colorIt % colors.size()];
            std::string cKey = "cluster_" + std::to_string(clusterID);
            j[eKey]["Hits"][cKey] = json::array();
            if (cl.getEnergy() > 1000) {
              clusterSize = 25.;
            } else if (cl.getEnergy() > 100) {
              clusterSize = 15.;
            } else {
              clusterSize = 5.;
            }
            // create centroid object
            cluster["originID"] = -1;
            cluster["pdgID"] = -1;
            cluster["energy"] = cl.getEnergy();
            cluster["color"] = hex;
            cluster["pos"] = { cl.getCentroidX(), cl.getCentroidY(), cl.getCentroidZ(),
                                clusterSize, clusterSize, clusterSize };
            j[eKey]["Hits"][cKey].push_back(cluster);
            if (includeEcalRecHits_) {
              for (auto const& clHitID : cl.getHitIDs()) {
                // map hit id to cluster it belongs to
                hitToCluster.insert({clHitID, clusterID});
              }
              if (cl.getHitIDs().size() == 1) {
                singleHitClusters++;
              }
            }
            clusterID++;
            colorIt++;
          } else {
            cluster["color"] = "0x000000";
            cluster["pos"] = { cl.getCentroidX(), cl.getCentroidY(), cl.getCentroidZ(),
                              singleClusterSize, singleClusterSize, singleClusterSize };
            j[eKey]["Hits"]["empty_clusters"].push_back(cluster);
          }
        }
      }

      // std::cout << "Single hit clusters: " << singleHitClusters << std::endl;
      
      if (includeEcalRecHits_) {

        std::string hit_coll_name;
        if (includeEcalClusters_) hit_coll_name = "clusterless_hits";
        else hit_coll_name = "ecal_rec_hits";
        j[eKey]["Hits"][hit_coll_name] = json::array();

        for (auto const& hit : ecalRecHits) {
          // if (!hit.isNoise()) {
            json h = json::object();
            h["type"] = "Box";
            h["energy"] = hit.getEnergy();
            if (includeGroundTruth_) {
              auto it = std::find_if(ecalSimHits.begin(), ecalSimHits.end(), [&hit](const auto& simHit) { return simHit.getID() == hit.getID(); });
              if (it != ecalSimHits.end()) {
                h["originID"] = json::array();
                for (int i = 0; i < it->getNumberOfContribs(); i++) {
                  auto c = it->getContrib(i);
                  h["originID"].push_back(c.originID);
                  h["pdgID"].push_back(c.pdgCode);
                }
              }
            }
            if (includeEcalClusters_) {
              auto it = hitToCluster.find(hit.getID());
              if (it != hitToCluster.end()) { // if hit is associated to a cluster
                auto id = it->second;
                // add hit to cluster collection
                // color will automatically be set to same as centroid
                std::string cKey = "cluster_" + std::to_string(id);
                h["pos"] = { hit.getXPos(), hit.getYPos(), hit.getZPos(),
                                clusterHitSize, clusterHitSize, clusterHitSize };
                j[eKey]["Hits"][cKey].push_back(h);
                continue;
              }
            }
            // if hit is not associated to a cluster
            h["color"] = "0xFF0000";
            h["pos"] = { hit.getXPos(), hit.getYPos(), hit.getZPos(),
                          clusterlessHitSize, clusterlessHitSize, clusterlessHitSize };
            j[eKey]["Hits"][hit_coll_name].push_back(h);
          // }
        }
      }
    }

    // // ----- TRACKS -----
    j[eKey]["Tracks"] = json::object();

    // GROUND TRUTH (SIMULATED) PATHS
    if (includeGroundTruth_) {
      j[eKey]["Tracks"]["ground_truth_tracks"] = json::array();
      auto particle_map{event.getMap<int, ldmx::SimParticle>("SimParticles")};
      for (const auto& it : particle_map) {
        json track = json::object();
        const auto& start = it.second.getVertex();
        const auto& end = it.second.getEndPoint();
        track["parentID"] = {it.second.getParents()};
        track["pos"] = { { start[0], start[1], start[2] },
                          { end[0], end[1], end[2] } };
        j[eKey]["Tracks"]["ground_truth_tracks"].push_back(track);
      }
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
