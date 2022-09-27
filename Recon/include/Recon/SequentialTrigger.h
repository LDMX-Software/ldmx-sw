/**
 * @file TriggerProcessor.h
 * @brief Class that provides a trigger decision for recon using a TriggerResult
 * object
 * @author Rory O'Dwyer, Stanford University
 * @author Lene Kristian Bryngemark, Stanford University
 */

#ifndef RECON_TRIGGER_SEQUENTIALTRIGGER_H_
#define RECON_TRIGGER_SEQUENTIALTRIGGER_H_

// LDMX
#include "Ecal/Event/EcalHit.h"
#include "Event/TriggerResult.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace recon {

/**
 * @class TriggerProcessor
 * @brief Provides a trigger decision for recon using a TriggerResult object.
 *
 * @note
 * TriggerProcessor takes in a set of parameters to be used in defining
 * the trigger algorithm. An event is passed to the processor and the relevant
 * algorithms are then run on the event (ECAL layer sum). A trigger decision is
 * executed and the decision along with the algorithm name and relevant
 * variables are stored in a TriggerResult object which is added to the
 * collection.
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
  virtual ~SequentialTrigger() {}

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters& parameters) final override;

  /**
   * Run the trigger algorithm and create a TriggerResult
   * object to contain info about the trigger decision
   * such as pass/fail, number of saved variables,
   * etc.
   * param event The event to run trigger algorithm on.
   */
  virtual void produce(framework::Event& event);

 private:  
  /** The name of the input collection of triggers */
  std::vector<std::string> trigger_list_;

  /** pass name of the triggers, should be uniform */
  std::vector<std::string> trigger_passNames_;

  /** integer list corresponding to decimal values of binary masks which lead
   * to a pass */
  std::vector<int> pass_masks_;
  
  /** options to run without masks*/
  bool doOR_;
  bool doAND_;
};

}  // namespace recon

#endif
