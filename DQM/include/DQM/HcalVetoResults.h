#ifndef DQM_HCALVETORESULTS_H
#define DQM_HCALVETORESULTS_H

#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace dqm {

/**
 * @class HcalVetoResults
 * @brief Generate histograms to check the final decisions made in the Hcal veto
 */
class HcalVetoResults : public framework::Analyzer {
 public:
  /**
   * Constructor
   *
   * Blank Analyzer constructor
   */
  HcalVetoResults(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process) {}

  /**
   * Destructor
   *
   * Blank Analyzer Destructor
   */
  ~HcalVetoResults() = default;

  /**
   * Input python configuration parameters
   */
  virtual void configure(framework::config::Parameters& ps);

  /**
   * Things to do only on process start
   */
  virtual void onProcessStart();
  
  /**
   * Fills histograms
   */
  virtual void analyze(const framework::Event& event);

 private:
  /// Collection Name for veto object
  std::string hcal_veto_name_;

  /// Pass Name for veto object
  std::string hcal_veto_pass_;
};
}  // namespace dqm

#endif /* DQM_HCALVETORESULTS_H */
