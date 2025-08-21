#ifndef DQM_TSDIGIVERIFIER_H
#define DQM_TSDIGIVERIFIER_H

#include <algorithm>

// LDMX Framework
#include "DetDescr/TrigScintID.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "TrigScint/Event/TrigScintHit.h"

namespace dqm {

/**
 * @class TrigScintDigiVerifier
 * @brief Generate histograms to check digi pipeline performance
 */
class TrigScintDigiVerifier : public framework::Analyzer {
 public:
  /**
   * Constructor
   *
   * Blank Analyzer constructor
   */
  TrigScintDigiVerifier(const std::string& name, framework::Process& process)
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
  std::string ts_simhit_coll_;

  /// Pass Name for SimHits
  std::string ts_simhit_pass_;

  /// Collection Name for digis
  std::string ts_digi_coll_;

  /// Pass Name for digis
  std::string ts_digi_pass_;
};
}  // namespace dqm

#endif /* DQM_TSDIGIVERIFIER_H */
