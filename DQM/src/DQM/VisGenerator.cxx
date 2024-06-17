#include "DQM/VisGenerator.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "Ecal/Event/EcalHit.h"
#include "Ecal/Event/EcalCluster.h"

#include <fstream>

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
    std::string eventJSON = "";
    int colorIt = 0;
    bool first = true;

    // ----- EVENT HEADER -----
    if (firstEvent) firstEvent = false;
    else eventJSON += ",";
    // Corresponds to LDMX_event
    eventJSON += "\n" + l1 + "\"EVENT_KEY_" + std::to_string(event.getEventNumber()) + "\": {\n"; // opens event
    // Event info (event number + run number)
    eventJSON += l2 + "\"event number\": " + std::to_string(event.getEventNumber()) + ",\n"
          + l2 + "\"run number\": " + std::to_string(runNbr_) + ",\n";
    
    // ----- HITS -----
    eventJSON += l2 + "\"Hits\": {\n";

    // RECONSTRUCTED ECAL HITS
    // std::vector<ldmx::EcalHit> ecalRecHits = event.getCollection<ldmx::EcalHit>(ecalRecHitColl_, ecalRecHitPass_);
    // eventJSON += l3 + "\"" + ecalRecHitColl_ + "\": [\n"; // opens ecal rechits
    // double boxSize = 1.0;
    // bool first = true;
    // // std::string hex = "\"0xE9E317\"";
    // std::string& hex =  colors[colorIt];
    // for(auto const& hit : ecalRecHits){
    //   // each hit is hit object of type Box, with "pos": [x,y,z]
    //   if (!hit.isNoise()) {
    //     if (first) first = false;
    //     else eventJSON += ",\n";
    //     eventJSON += l4 + "{ \"color\":" + hex + ", \"type\": \"Box\", "
    //             + "\"pos\": ["
    //             + std::to_string(hit.getXPos()) + ","
    //             + std::to_string(hit.getYPos()) + ","
    //             + std::to_string(hit.getZPos()) + ","
    //             + std::to_string(boxSize) + ","
    //             + std::to_string(boxSize) + ","
    //             + std::to_string(boxSize) + "]}";
    //   }
    // }
    // eventJSON += "\n" + l3 + "],\n";
    // colorIt++;
    // // CLUSTER CENTROIDS
    // std::vector<ldmx::EcalCluster> ecalClusters = event.getCollection<ldmx::EcalCluster>(ecalClusterColl_, ecalClusterPass_);
    // eventJSON += l3 + "\"" + ecalClusterColl_ + "\": [\n"; // opens ecal rechits
    // first = true;
    // boxSize = 5.0;
    // // hex = "\"0xE9E317\"";
    // hex =  colors[colorIt];
    // for (auto const& cl : ecalClusters) {
    //   if (first) first = false;
    //   else eventJSON += ",\n";
    //   eventJSON += l4 + "{ \"color\":" + hex + ", \"type\": \"Box\", "
    //             + "\"pos\": ["
    //             + std::to_string(cl.getCentroidX()) + ","
    //             + std::to_string(cl.getCentroidY()) + ","
    //             + std::to_string(cl.getCentroidZ()) + ","
    //             + std::to_string(boxSize) + ","
    //             + std::to_string(boxSize) + ","
    //             + std::to_string(boxSize) + "]}";
    // }
    // eventJSON += "\n" + l3 + "]";

    // ----- CLUSTER-RECHIT CONNECTION -----
    std::vector<ldmx::EcalCluster> ecalClusters = event.getCollection<ldmx::EcalCluster>(ecalClusterColl_, ecalClusterPass_);
    std::vector<ldmx::EcalHit> ecalRecHits = event.getCollection<ldmx::EcalHit>(ecalRecHitColl_, ecalRecHitPass_);

    std::unordered_map<int, int> hitToCluster;
    hitToCluster.reserve(ecalRecHits.size());

    std::unordered_map<int, std::string> clusterToJSON;
    // clusterToJSON.reserve(ecalClusters.size());

    int clusterID = 1;
    int clusterSize = 15.0;
    int clusterHitSize = 5.0;
    int hitBoxSize = 2.0;

    std::string clusterless = l3 + "\"clusterless\": [\n";
    first = true;

    for (auto const& cl : ecalClusters) {
      if (cl.getHitIDs().size() != 0) { // if cluster is larger than just one hit
        std::string& hex = colors[colorIt % colors.size()];
        std::string coll = l3 + "\"cluster_" + std::to_string(clusterID) + "\": [\n";
        coll += l4 + "{ \"color\":" + hex + ", \"type\": \"Box\", "
                + "\"pos\": ["
                + std::to_string(cl.getCentroidX()) + ","
                + std::to_string(cl.getCentroidY()) + ","
                + std::to_string(cl.getCentroidZ()) + ","
                + std::to_string(clusterSize) + ","
                + std::to_string(clusterSize) + ","
                + std::to_string(clusterSize) + "]}";
        for (auto const& clHitID : cl.getHitIDs()) {
          // tried out finding multiplicities, did not seem to happen
          hitToCluster.insert({clHitID, clusterID});
        }
        clusterToJSON.insert({clusterID, coll});
        clusterID++;
        colorIt++;
      } else {
        if (first) first = false;
        else clusterless += ",\n";
        clusterless += l4 + "{ \"color\": \"0xFF0000\", \"type\": \"Box\", "
                + "\"pos\": ["
                + std::to_string(cl.getCentroidX()) + ","
                + std::to_string(cl.getCentroidY()) + ","
                + std::to_string(cl.getCentroidZ()) + ","
                + std::to_string(hitBoxSize) + ","
                + std::to_string(hitBoxSize) + ","
                + std::to_string(hitBoxSize) + "]}";
      }
    }

    for (auto const& hit : ecalRecHits) {
      auto& id = hitToCluster[hit.getID()];
      if (id != 0) {
        std::string& json = clusterToJSON[id];
        json += ",\n" + l4 + "{ \"type\": \"Box\", "
                + "\"pos\": ["
                + std::to_string(hit.getXPos()) + ","
                + std::to_string(hit.getYPos()) + ","
                + std::to_string(hit.getZPos()) + ","
                + std::to_string(clusterHitSize) + ","
                + std::to_string(clusterHitSize) + ","
                + std::to_string(clusterHitSize) + "]}";
      }
    }

    eventJSON += clusterless + "\n" + l3 + "]";
    for (auto const& kv : clusterToJSON) {
      eventJSON += ",\n" + kv.second + "\n" + l3 + "]";
    }
    // Add other hit formats here

    eventJSON += "\n" + l2 + "},\n"; // closes hits

    // ----- TRACKS -----
    eventJSON += l2 + "\"Tracks\": {\n"; // opens tracks

    // GROUND TRUTH (SIMULATED) PATHS
    eventJSON += l3 + "\"GroundTruthTracks\": [\n"; // opens ground truth tracks
    auto particle_map{event.getMap<int, ldmx::SimParticle>("SimParticles")};
    first = true;
    // hex = "\"0xff00ff\"";
    for (const auto& it : particle_map) {
      if (first) first = false;
      else eventJSON += ",\n";
      std::vector<double> start = it.second.getVertex();
      std::vector<double> end = it.second.getEndPoint();
      eventJSON += l4 + "{ \"pos\": [["
              + std::to_string(start[0]) + ","
              + std::to_string(start[1]) + ","
              + std::to_string(start[2]) + "],"
            + "[" 
              + std::to_string(end[0]) + ","
              + std::to_string(end[1]) + ","
              + std::to_string(end[2]) + "]"
            + "]}";
    }
    eventJSON += "\n" + l3 + "]"; // closes ground truth tracks

    // Add other tracks here

    eventJSON += "\n" + l2 + "}"; // closes tracks

    // Add other types of objects here (vertices, clusters...)

    eventJSON += "\n" + l1 + "}"; // closes event
    JSONstr += eventJSON;
    return;
  }

void VisGenerator::onProcessEnd(){
    std::ofstream file(filename_);
    file << JSONstr + "\n}" << std::endl;
    file.close();
    return;
};

}

DECLARE_ANALYZER_NS(dqm, VisGenerator)
