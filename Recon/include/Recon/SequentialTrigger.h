/**
 * @file SequentialTrigger.h
 * @brief Class that provides a trigger skimming decision from multiple Triggers
 * based on either AND or OR.
 * object
 * @author Rory O'Dwyer, Stanford University
 * @author Lene Kristian Bryngemark, Stanford University
 */

#ifndef RECON_TRIGGER_SEQUENTIALTRIGGER_H_
#define RECON_TRIGGER_SEQUENTIALTRIGGER_H_

// LDMX
#include "Event/TriggerResult.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace recon {

/**
 * @class SequentialTrigger
 * @brief Class that provides a trigger skimming decision from multiple Triggers
 * based on either AND or OR.
 * *
 * @note
 * TriggerProcessor takes in a set of parameters to determine whether, upon an
 * input collection of triggers, an OR or AND descision will be made. Each
 * trigger layer is obtained after TriggerProcessor is run, and then running
 * through the array the first pass returns true for doOR_ and the first fail
 * false for doAND_. Once at worst all triggers are run through, we set the
 * keep event flag to the resultant doOR or doAND result. If doVAL is set to
 * true, we produce and output collection with this boolean value.
 */
class SequentialTrigger : public framework::Producer {
 public:
  /**
   * Class constructor.
   */
  SequentialTrigger(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  /**
   * Class destructor.
   */
    virtual ~SequentialTrigger() = default;

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters& parameters) override;

  /**
   * Run the doOR or doAND check and create a SequentialTrigger
   * object to contain the pass boolean value. Also sets the
   * Storage Hint.
   * param event The event to run skimmer on.
   */
  void produce(framework::Event& event) override;

 private:
  /** The name of the input collection of triggers */
  std::vector<std::string> trigger_list_;

  /** pass name of the triggers */
  std::vector<std::string> trigger_passNames_;

  /** options to enable OR or AND skimming*/
  bool doOR_;
  bool doAND_;

  /**
   * enables a output collection with the keep tag for the purposes of
   * validation
   * */
  bool doVAL_;
};

}  // namespace recon

#endif
