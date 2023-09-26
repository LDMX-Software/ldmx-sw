/**
 * @file StorageControl.h
 * @brief Definitions related to event storage control from an EventProcessor
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef FRAMEWORK_STORAGECONTROL_H_
#define FRAMEWORK_STORAGECONTROL_H_

#include <string>
#include <vector>

namespace framework {

typedef enum enum_StorageControlHint {
  hint_Undefined = 0,
  hint_NoOpinion,
  hint_shouldKeep = 10,
  hint_mustKeep = 11,
  hint_shouldDrop = 20,
  hint_mustDrop = 21
} StorageControlHint;

/**
 * @class StorageControl
 * @brief Class which encapsulates storage control functionality, used by the
 * Process class
 *
 * Any EventProcessor can provide a hint as to whether a given
 * event should be kept or dropped.  The hint is cached in the
 * StorageControl object until the end of the event.  At that
 * point, the process queries the StorageControl to determine if
 * the event should be stored in the output file.
 */
class StorageControl {
 public:
  /** Set the default state */
  void setDefaultKeep(bool keep) { defaultIsKeep_ = keep; }

  /**
   * Reset the event-by-event state
   */
  void resetEventState();

  /**
   * Add a storage hint for a given module
   * @param processor_name Name of the event processor
   * @param controlhint The storage control hint to apply for the given event
   * @param purposeString A purpose string which can be used in the skim control
   * configuration
   */
  void addHint(const std::string& processor_name,
               framework::StorageControlHint hint,
               const std::string& purposeString);

  /**
   * Add a rule
   * @param processor_pattern Regex pattern to compare with event processor
   * @param purpose_pattern Regex pattern to compare with the purpose string
   */
  void addRule(const std::string& processor_pat,
               const std::string& purpose_pat);

  /**
   * Determine if the current event should be kept, based on the defined rules
   *
   * Separating the 'must' keywords into their own tier helps give them
   * greater importance that reflects their name. In order, the decision
   * follows the criteria below only using hints that match one of the
   * listening rules configured for the process.
   * 
   * 1. If any hint is mustDrop, drop the event.
   * 2. If any hint is mustKeep (without any mustDrop), keep the event.
   * 3. Simple majority decision of votes made for keep and drop.
   * 4. The default decision is made in the event of a tie (or no votes).
   *
   * @param[in] event_completed did we complete processing of the current event?
   * @returns true if we should store the current event into the output file
   */
  bool keepEvent(bool event_completed) const;

 private:
  /**
   * Default state for storage control
   */
  bool defaultIsKeep_{true};

  /**
   * Structure to hold hints
   */
  struct Hint {
    /**
     * Event Processor name
     */
    std::string evpName_;
    /**
     * Hint level
     */
    StorageControlHint hint_;
    /**
     * Purpose string, if used
     */
    std::string purpose_;
  };

  /**
   * Collection of hints from the event processors
   */
  std::vector<Hint> hints_;

  /**
   * Structure to hold rules
   *
   * Eventually need a cleanup function to remove the compiled regex buffers,
   * but only in the _StorageControl_ destructor, not during regular vector
   * operations.
   */
  struct Rule {
    bool matches(const Hint& h) const;

    /**
     * Event Processor Regex
     */
    std::string evpNamePattern_;

    /**
     * Purpose string Regex
     */
    std::string purposePattern_;

    /**
     * Compiled event processor regex
     */
    void* evpNameRegex_{0};

    /**
     * Compiled event processor regex
     */
    void* purposeRegex_{0};
  };

  /**
   * Collection of hints from the event processors
   */
  std::vector<Rule> rules_;
};
}  // namespace framework

#endif
