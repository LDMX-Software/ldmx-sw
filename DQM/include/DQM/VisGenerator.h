#ifndef DQM_VISGENERATOR_H
#define DQM_VISGENERATOR_H

//LDMX Framework
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

// JSON
#include "DQM/json.hpp"

namespace dqm {

/**
 * @class VisGenerator
 * @brief Generates JSON file of event data for Phoenix visualisation
 */

class VisGenerator : public framework::Analyzer {
  public:
    VisGenerator(const std::string& name, framework::Process& process) : Analyzer(name, process) {}

    virtual void configure(framework::config::Parameters& ps);

    virtual void analyze(const framework::Event& event);

    void ecalClusterRecHit(const framework::Event& event, const std::string& eKey);

    void groundTruthTracks(const framework::Event& event, const std::string& eKey);

    void extractLayers(const framework::Event& event, const std::string& eKey);

    // void caloCells(const framework::Event& event, const std::string& eKey);

    virtual void onProcessEnd();

  private:
    // Include ground truth (simulated) info
    bool includeGroundTruth_;

    // Simulated info has contribs with originID (not available by default)
    bool originIdAvailable_;

    // Number of electrons in simulation
    int nbrOfElectrons_;

    // Collection Name for SimHits
    std::string ecalSimHitColl_;

    // Pass Name for SimHits
    std::string ecalSimHitPass_;

    // Include ecal rec hits
    bool includeEcalRecHits_;

    // Collection Name for RecHits
    std::string ecalRecHitColl_;

    // Pass Name for RecHits
    std::string ecalRecHitPass_;

    // Include ecal clusters
    bool includeEcalClusters_;

    // Collection name for ecal clusters
    std::string ecalClusterColl_;

    // Pass name for ecal clusters
    std::string ecalClusterPass_;

    // Generate json file visualizing hit origins
    // NEEDS ORIGIN ID
    bool visHitOrigin_;
    std::string truthFilename_;

     // Generate json file visualizing ecal layers
    bool visLayers_;
    std::string layerFilename_;

    // Output filename
    std::string filename_;

    // Run number
    int runNbr_;
    
    nlohmann::json j;

    nlohmann::json truth;

    nlohmann::json layer;

    // nlohmann::json c;

    std::vector<std::string> colors { "0xFFB6C1", "0xFFA500", "0xFFFF00", 
                                      "0x7FFF00", "0x00FFFF", "0x663399"};

    std::vector<std::string> colorstrings { "pink", "orange", "yellow", 
                                      "green", "blue", "purple"};
};

}

#endif
