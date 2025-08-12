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
  std::string input_col_;  // input coll. containing 2-hit clusters (standard)
  std::string input_pass_name_{""};
  //  std::string wideInputCol_;   // input coll. containing 3-hit clusters
  //  std::string wideInputPassName_{inputPassName};  // default to same pass

  // plotting stuff
  int n_channels_{16};
  // match nchan above
  TH2F* h_n3_n2_;
  TH2F* h_n3_n1_;
  TH2F* h_n2_n1_;
  TH1F* h_n_clusters_;
  TH1F* h_n_hits_;
  // TH1F* hNhitsInClusters;
  TH1F* h_p_ein_hits_[16];
  TH1F* h_p_ein_clusters_[16];
  TH1F* h_delta_centroids_;
  TH2F* h_delta_vs_seed_;
};
}  // namespace trigscint

#endif /* TRIGSCINT_TESTBEAMCLUSTERANALYZER_H */
