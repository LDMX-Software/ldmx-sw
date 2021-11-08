#ifndef DQM_TRIGGER_H
#define DQM_TRIGGER_H

// LDMX Framework
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/EventProcessor.h"  //Needed to declare processor

namespace dqm {

/**
 * @class Trigger
 * @brief Generate histograms to check digi pipeline performance
 */
class Trigger : public framework::Analyzer {
 public:
  /**
   * Constructor
   *
   * Blank Analyzer constructor
   */
  Trigger(const std::string& name, framework::Process& process)
      : framework::Analyzer(name, process) {}

  /**
   * Input python configuration parameters
   */
  virtual void configure(framework::config::Parameters& ps);

  /** 
   * Method executed before processing of events begins. 
   */
  virtual void onProcessStart();

  
  /**
   * Fills histograms
   */
  virtual void analyze(const framework::Event& event);

 private:
  /// Trigger collection name 
  std::string trigger_collName_;

  /// Trigger collection pass name
  std::string trigger_passName_;

};
}  // namespace dqm

#endif /* DQM_TRIGGER_H */
