#ifndef DQM_VISGENERATOR_H
#define DQM_VISGENERATOR_H

//LDMX Framework
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

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

    virtual void onProcessEnd();

  private:
    // Collection Name for SimHits
    std::string ecalSimHitColl_;

    // Pass Name for SimHits
    std::string ecalSimHitPass_;

    // Collection Name for RecHits
    std::string ecalRecHitColl_;

    // Pass Name for RecHits
    std::string ecalRecHitPass_;

    // Collection name for Ecal Clusters
    std::string ecalClusterColl_;

    std::string ecalClusterPass_;

    // Output filename
    std::string filename_;

    // Run number
    int runNbr_;

    bool firstEvent{true};
    std::string JSONstr{"{"};

    // Indentation levels for JSON file; two spaces per level
    std::string l1{"  "};
    std::string l2{"    "};
    std::string l3{"      "};
    std::string l4{"        "};

    std::vector<std::string> colors { "\"0xFFBF00\"", "\"0xE9E317\"", "\"0xE9E317\"", "\"0xFF00FF\"", "\"0x#FF7F50\"", "\"0xDE3163\"", "\"0x#9FE2BF\"", "\"0x#CCCCFF\""};
};

}

#endif
