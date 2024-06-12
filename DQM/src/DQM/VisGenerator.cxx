#include "DQM/VisGenerator.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "Ecal/Event/EcalHit.h"

// #include <iostream>
#include <fstream>
// #include <algorithm>

namespace dqm {

  void VisGenerator::configure(framework::config::Parameters& ps) {
    ecalSimHitColl_ = ps.getParameter<std::string>("ecalSimHitColl");
    ecalSimHitPass_ = ps.getParameter<std::string>("ecalSimHitPass");
    ecalRecHitColl_ = ps.getParameter<std::string>("ecalRecHitColl");
    ecalRecHitPass_ = ps.getParameter<std::string>("ecalRecHitPass");
    filename_ = ps.getParameter<std::string>("filename");
    runNbr_ = ps.getParameter<int>("runNumber");
    return;
  }

  void VisGenerator::analyze(const framework::Event& event) {
    std::string eventJSON = "";

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
    std::vector<ldmx::EcalHit> ecalRecHits = event.getCollection<ldmx::EcalHit>(ecalRecHitColl_, ecalRecHitPass_);
    eventJSON += l3 + "\"" + ecalRecHitColl_ + "\": [\n"; // opens ecal rechits
    double boxSize = 5.0;
    bool firstHit = true;
    for(auto const& hit : ecalRecHits){
      // each hit is hit object of type Box, with "pos": [x,y,z]
      if (!hit.isNoise()) {
        if (firstHit) firstHit = false;
        else eventJSON += ",\n";
        eventJSON += l4 + "{ \"type\": \"Box\", \"pos\": ["
                + std::to_string(hit.getXPos()) + ","
                + std::to_string(hit.getYPos()) + ","
                + std::to_string(hit.getZPos()) + ","
                + std::to_string(boxSize) + ","
                + std::to_string(boxSize) + ","
                + std::to_string(boxSize) + "]}";
      }
    }
    eventJSON += "\n" + l3 + "]";

    // Add other hit formats here

    eventJSON += "\n" + l2 + "},\n"; // closes hits

    // ----- TRACKS -----
    eventJSON += l2 + "\"Tracks\": {\n"; // opens tracks
    // GROUND TRUTH (SIMULATED) PATHS
    eventJSON += l3 + "\"GroundTruthTracks\": [\n"; // opens ground truth tracks
    auto particle_map{event.getMap<int, ldmx::SimParticle>("SimParticles")};
    bool firstTrack = true;
    for (const auto& it : particle_map) {
      if (firstTrack) firstTrack = false;
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
