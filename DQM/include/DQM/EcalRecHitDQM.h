#ifndef DQM_ECALRECHITDQM_H
#define DQM_ECALRECHITDQM_H

#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"

namespace dqm {

/**
 * Histograms of ECal rec hits that need no simulation truth,
 * e.g. for raw data: hits per event, amplitude in MIPs, time,
 * and the occupancy per layer and cell (checks the detector map).
 */
class EcalRecHitDQM : public framework::Analyzer {
 public:
  /// constructor
  EcalRecHitDQM(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process) {}

  /// configure the input names
  void configure(framework::config::Parameters& parameters) override;

  /// create the histograms
  void onProcessStart() override;

  /// fill the histograms
  void analyze(const framework::Event& event) override;

 private:
  /// name of the rec hit collection
  std::string rec_hit_coll_;
  /// pass of the rec hits
  std::string rec_hit_pass_;
  /// Si energy of one MIP [MeV], converts the hit amplitude to MIPs
  double mip_si_energy_;
};

}  // namespace dqm

#endif  // DQM_ECALRECHITDQM_H
