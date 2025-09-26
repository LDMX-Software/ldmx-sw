#ifndef DQM_TRKDEDXMASSESTFEATURES_H
#define DQM_TRKDEDXMASSESTFEATURES_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/EventProcessor.h"  //Needed to declare processor

namespace dqm {

/**
 * @class TrkDeDxMassEstFeatures
 * @brief Generate histograms to check tracker dE/dx mass estimate features
 */
class TrkDeDxMassEstFeatures : public framework::Analyzer {
 public:
  /**
   * Constructor
   *
   * Blank Analyzer constructor
   */
  TrkDeDxMassEstFeatures(const std::string& name, framework::Process& process)
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
  /// Collection Name for mass estimate object
  std::string mass_estimate_name_;

  /// Pass Name for mass estimate object
  std::string mass_estimate_pass_;
};
}  // namespace dqm

#endif /* DQM_TRKDEDXMASSESTFEATURES_H */
