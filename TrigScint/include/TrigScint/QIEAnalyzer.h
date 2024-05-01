/**
 * @file QIEAnalyzer.h
 * @brief
 * @author
 */

#ifndef TRIGSCINT_QIEANALYZER_H
#define TRIGSCINT_QIEANALYZER_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "TH1.h"
#include "TH2.h"
#include "TrigScint/Event/EventReadout.h"

namespace trigscint {

/**
 * @class QIEAnalyzer
 * @brief
 */
class QIEAnalyzer : public framework::Analyzer {
 public:
  QIEAnalyzer(
      const std::string& name,
      framework::Process& process);  // : framework::Analyzer(name, process) {}
  virtual ~QIEAnalyzer() = default;
  void configure(framework::config::Parameters& parameters) override;

  void analyze(const framework::Event& event) override;

  void onProcessStart() override;

  void onProcessEnd() override;

  float convertToID(float yVal) { return (yVal + yOffset_) * yToIDfactor_; }

 private:
  std::vector<std::vector<TH1F*> > vChargeVsTime;

  // configurable parameters
  std::string inputCol_;
  std::string inputPassName_{""};
  std::vector<double> peds_;
  std::vector<double> gain_;
  int startSample_{0};

  // plotting stuff
  int evNb;
  int nEv{200};
  int nChannels{16};
  // int nTrkMax{100};

  // match nev, nchan above
  TH1F* hOut[200][16];
  TH1F* hPE[16];
  TH2F* hPEvsT[16];
  TH2F* hPedSubtractedAvgQvsT[16];
  TH2F* hPedSubtractedTotQvsPed[16];
  TH2F* hPedSubtractedTotQvsN[16];
  TH2F* hTotQvsPed[16];
  TH2F* hPedSubtractedPEvsN[16];
  TH2F* hPedSubtractedPEvsT[16];
  TH2F* hAvgQvsT[16];

  TH2F* hTDCfireChanvsEvent;
  double yOffset_{35.};
  double yToIDfactor_{50. / 80.};
};
}  // namespace trigscint

#endif /* TRIGSCINT_QIEANALYZER_H */
