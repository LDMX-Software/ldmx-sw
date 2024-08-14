#ifndef DQM_ECALMIPTRACKINGFEATURES_H
#define DQM_ECALMIPTRACKINGFEATURES_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/EventProcessor.h"  //Needed to declare processor

namespace dqm {

/**
 * @class EcalMipTrackingFeatures
 * @brief Generate histograms to check MIP tracking features
 */
class EcalMipTrackingFeatures : public framework::Analyzer {
 public:
  /**
   * Constructor
   *
   * Blank Analyzer constructor
   */
  EcalMipTrackingFeatures(const std::string& name, framework::Process& process)
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

#endif /* DQM_ECALMMIPTRACKINGFEATURES_H */
