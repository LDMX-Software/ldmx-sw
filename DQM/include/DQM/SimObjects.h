#ifndef DQM_SIMOBJECTS_H
#define DQM_SIMOBJECTS_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/EventProcessor.h"  //Needed to declare processor

namespace dqm {

/**
 * @class SimObjects
 * @brief Generate histograms to check simulation output
 */
class SimObjects : public framework::Analyzer {
 public:
  /**
   * Constructor
   *
   * Blank Analyzer constructor
   */
  SimObjects(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process) {}

  /**
   * Input python configuration parameters
   */
  virtual void configure(framework::config::Parameters& ps);

  /**
   * Construct histograms depending on which objects are requested.
   */
  virtual void onProcessStart();

  /**
   * Fills histograms
   */
  virtual void analyze(const framework::Event& event);

 private:
  /// SimParticles collection name
  std::string sim_particles_name_;

  /// Calorimeter Hit collection names
  std::vector<std::string> sim_calorimeter_names_;

  /// Tracker hit collection names
  std::vector<std::string> sim_tracker_names_;

  /// Pass Name for sim objects
  std::string sim_pass_;
};
}  // namespace dqm

#endif /* DQM_SIMOBJECTS_H */
