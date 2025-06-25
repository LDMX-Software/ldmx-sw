#ifndef DQM_ECALPNETVETORESULTS_H
#define DQM_ECALPNETVETORESULTS_H

#include "Ecal/Event/EcalVetoResult.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"


namespace dqm {

/**
 * @class EcalPnetVetoResults
 * @brief Generate histograms to check the final decisions made in the ECAL veto
 */
class EcalPnetVetoResults : public framework::Analyzer {
 public:
  /**
   * Constructor
   *
   * Blank Analyzer constructor
   */
  EcalPnetVetoResults(const std::string& name, framework::Process& process)
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
  std::string ecal_pnet_veto_name_;

  /// Pass Name for veto object
  std::string ecal_pnet_veto_pass_;
};
}  // namespace dqm

#endif /* DQM_ECALPNETVETORESULTS_H */
