#ifndef DQM_ECALWABRECRESULTS_H
#define DQM_ECALWABRECRESULTS_H

#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace dqm {

/**
 * @class EcalWABRecResults
 * @brief Generate histograms to check WAB-related angular reconstruction
 */
class EcalWABRecResults : public framework::Analyzer {
 public:
  /**
   * Constructor
   *
   * Blank Analyzer constructor
   */
  EcalWABRecResults(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process) {}

  /**
   * Input python configuration parameters
   */
  virtual void configure(framework::config::Parameters& ps) override;

  /**
   * Fills histograms
   */
  virtual void analyze(const framework::Event& event) override;

 private:
  /// Collection Name for WABRec object
  std::string ecal_wab_rec_name_;

  /// Pass Name for WABRec object
  std::string ecal_wab_rec_pass_;
};
}  // namespace dqm

#endif /* DQM_ECALWABRECRESULTS_H */
