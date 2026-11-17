#ifndef DQM_ECONDDIGIDQM_H
#define DQM_ECONDDIGIDQM_H

#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"

namespace dqm {

/**
 * Histograms of the output of packing::ECONDDecoder
 *
 * Fills the corruption flags and header data from the ECONDEventInfo
 * and the channel measurements from the HgcrocDigiCollection.
 * The measurements are taken from the sample word bits following the
 * HGCROC convention (bit 31 = Tc, bit 30 = Tp).
 */
class ECONDDigiDQM : public framework::Analyzer {
 public:
  /// constructor
  ECONDDigiDQM(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process) {}

  /// configure the input names
  void configure(framework::config::Parameters& parameters) override;

  /// create the histograms
  void onProcessStart() override;

  /// fill the histograms
  void analyze(const framework::Event& event) override;

 private:
  /// name of the digi collection
  std::string digi_name_;
  /// name of the ECONDEventInfo
  std::string info_name_;
  /// pass of the decoder
  std::string input_pass_;
};

}  // namespace dqm

#endif  // DQM_ECONDDIGIDQM_H
