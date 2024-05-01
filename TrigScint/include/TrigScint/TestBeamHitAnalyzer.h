/**
 * @file TestBeamHitAnalyzer.h
 * @brief
 * @author
 */

#ifndef TRIGSCINT_TESTBEAMHITANALYZER_H
#define TRIGSCINT_TESTBEAMHITANALYZER_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "TH1.h"
#include "TH2.h"
#include "TrigScint/Event/TestBeamHit.h"

namespace trigscint {

/**
 * @class TestBeamHitAnalyzer
 * @brief
 */
class TestBeamHitAnalyzer : public framework::Analyzer {
 public:
  TestBeamHitAnalyzer(
      const std::string& name,
      framework::Process& process);  // : framework::Analyzer(name, process) {}
  virtual ~TestBeamHitAnalyzer() = default;
  void configure(framework::config::Parameters& parameters) override;

  void analyze(const framework::Event& event) override;

  void onProcessStart() override;

  void onProcessEnd() override;

 private:
  std::vector<std::vector<TH1F*> > vChargeVsTime;

  // configurable parameters
  std::string inputCol_;
  std::string inputPassName_{""};
  std::vector<double> peds_;
  int startSample_{0};

  // plotting stuff
  int evNb;
  int nEv{200};
  int nChannels{16};
  // int nTrkMax{100};
  TH2F* hEvDisp;
  TH2F* hEvDispPE;

  int fillNb{0};

  // match nev, nchan above
  TH1F* hOut[200][16];

  TH1F* hPE[16];
  TH1F* hPEinClusters[16];
  TH2F* hPEVsDelta[16];
  TH2F* hDeltaPEVsDelta[16];
  TH2F* hPEmaxVsDelta;
};
}  // namespace trigscint

#endif /* TRIGSCINT_TESTBEAMHITANALYZER_H */
