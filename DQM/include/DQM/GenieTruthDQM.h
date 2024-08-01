//
// Created by Wesley Ketchum on 8/1/24.
//

#ifndef DQM_GENIETRUTH_H
#define DQM_GENIETRUTH_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/EventProcessor.h"  //Needed to declare processor

namespace dqm {

/**
 * @class GenieTruthDQM
 * @brief Generate histograms/ntuple to extract genie output info
 */
class GenieTruthDQM : public framework::Analyzer {
 public:
  /**
   * Constructor
   *
   * Blank Analyzer constructor
   */
  GenieTruthDQM(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process) {}

  /**
   * Input python configuration parameters
   */
  virtual void configure(framework::config::Parameters& ps);

  /**
   * Construct histograms/ntuples
   */
  virtual void onProcessStart();

  /**
   * Grab the run number...
   */
  virtual void onNewRun(const ldmx::RunHeader &runHeader);

  /**
   * Fills histograms/ntuples
   */
  virtual void analyze(const framework::Event& event);

 private:
  /// Pass Name for genie objects
  std::string hepmc3CollName_;
  std::string hepmc3PassName_;

  int run_number_;
};
}  // namespace dqm


#endif  // DQM_GENIETRUTH_H
