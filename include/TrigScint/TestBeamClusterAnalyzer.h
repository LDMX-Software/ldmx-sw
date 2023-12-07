/**
 * @file TestBeamClusterAnalyzer.h
 * @brief
 * @author
 */

#ifndef TRIGSCINT_TESTBEAMCLUSTERANALYZER_H
#define TRIGSCINT_TESTBEAMCLUSTERANALYZER_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "TH1.h"
#include "TH2.h"
#include "TrigScint/Event/TrigScintCluster.h"

namespace trigscint {

/**
 * @class TestBeamClusterAnalyzer
 * @brief
 */
class TestBeamClusterAnalyzer : public framework::Analyzer {
 public:
  TestBeamClusterAnalyzer(
      const std::string& name,
      framework::Process& process);  // : framework::Analyzer(name, process) {}
  virtual ~TestBeamClusterAnalyzer() = default;
  void configure(framework::config::Parameters& parameters) override;

  void analyze(const framework::Event& event) override;

  void onProcessStart() override;

  void onProcessEnd() override;

 private:

  // configurable parameters
  std::string inputCol_;       // input coll. containing 2-hit clusters (standard)
  std::string inputPassName_{""};
  //  std::string wideInputCol_;   // input coll. containing 3-hit clusters 
  //  std::string wideInputPassName_{inputPassName};  // default to same pass 

  // plotting stuff
  int nChannels{16};
  // match nchan above
  TH2F* hN3N2;
  TH2F* hN3N1;
  TH2F* hN2N1;
  TH1F* hNClusters;
  TH1F* hNHits;
  TH1F* hNhitsInClusters;
  TH1F* hPEinHits[16];
  TH1F* hPEinClusters[16];
  TH1F* hDeltaCentroids;
  TH2F* hDeltaVsSeed;

};
}  // namespace trigscint

#endif /* TRIGSCINT_TESTBEAMCLUSTERANALYZER_H */
