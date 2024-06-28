#ifndef DQM_ECALCLUSTERANALYZER_H
#define DQM_ECALCLUSTERANALYZER_H

//LDMX Framework
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace ecal {

/**
 * @class EcalClusterAnalyzer
 * @brief
 */

class EcalClusterAnalyzer : public framework::Analyzer {
  public:
    EcalClusterAnalyzer(const std::string& name, framework::Process& process) : Analyzer(name, process) {}
    
    virtual void configure(framework::config::Parameters& ps);

    virtual void analyze(const framework::Event& event);

  private:
    // Collection Name for SimHits
    std::string ecalSimHitColl_;

    // Pass Name for SimHits
    std::string ecalSimHitPass_;

    // Collection Name for RecHits
    std::string recHitCollName_;

    // Pass Name for RecHits
    std::string recHitPassName_;

    // Collection Name for clusters
    std::string clusterCollName_;

    // Pass Name for clusters
    std::string clusterPassName_;

    // int depth_;
};

}

#endif
