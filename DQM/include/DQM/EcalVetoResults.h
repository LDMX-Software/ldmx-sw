#ifndef DQM_ECALVETORESULTS_H
#define DQM_ECALVETORESULTS_H

#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace dqm {

/**
 * @class EcalVetoResults
 * @brief Generate histograms to check the final decisions made in the ECAL veto
 */
class EcalVetoResults : public framework::Analyzer {
 public:
  /**
   * Constructor
   *
   * Blank Analyzer constructor
   */
  EcalVetoResults(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process) {}

  /**
   * Input python configuration parameters
   */
  virtual void configure(framework::config::Parameters& ps);

  /**
   * Fills histograms
   */
  virtual void analyze(const framework::Event& event);

 private:
  /// Collection Name for veto object
  std::string ecal_veto_name_;

  /// Pass Name for veto object
  std::string ecal_veto_pass_;
};
}  // namespace dqm

#endif /* DQM_ECALVETORESULTS_H */
