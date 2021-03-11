#ifndef DQM_ECALDIGIVERIFIER_H
#define DQM_ECALDIGIVERIFIER_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/EventProcessor.h"  //Needed to declare processor

namespace dqm {

/**
 * @class EcalDigiVerifier
 * @brief Generate histograms to check digi pipeline performance
 */
class EcalDigiVerifier : public framework::Analyzer {
 public:
  /**
   * Constructor
   *
   * Blank Analyzer constructor
   */
  EcalDigiVerifier(const std::string& name, framework::Process& process)
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
  /// Collection Name for SimHits
  std::string ecalSimHitColl_;

  /// Pass Name for SimHits
  std::string ecalSimHitPass_;

  /// Collection Name for RecHits
  std::string ecalRecHitColl_;

  /// Pass Name for RecHits
  std::string ecalRecHitPass_;
};
}  // namespace dqm

#endif /* DQM_ECALDIGIVERIFIER_H */
