/**
 * @file StorageControl.h
 * @brief Definitions related to event storage control from an EventProcessor
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef FRAMEWORK_STORAGECONTROL_H_
#define FRAMEWORK_STORAGECONTROL_H_

#include <regex>
#include <string>
#include <vector>

namespace framework {

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
  /**
   * Hints that can be provided by processors to the storage controller
   *
   * Integer values of the hints are currently not used for anything,
   * although one could imagine a "weighting" system being implemented
   * where different Hints are weighted based on how "strong" the hint is.
   */
  enum class Hint {
    NoOpinion = 0,
    Undefined = -1,
    ShouldKeep = 1,
    MustKeep = 10,
    ShouldDrop = 2,
    MustDrop = 20
  };  // enum Hint

 public:
  /** Set the default state */
  void setDefaultKeep(bool keep) { defaultIsKeep_ = keep; }

  /**
   * Reset the event-by-event state
   */
  void resetEventState();

  /**
   * Add a storage hint for a given processor
   *
   * This is the function eventually called when a processor uses 
   * EventProcessor::setStorageHint during processing.
   * When a hint is added, we check it against our configured listening rules.
   * If the hint does not match any of our listening rules, then we simply
   * ignore it by not adding it to our internal cache that will be used later
   * when deciding to keep the event.
   *
   * @param processor_name Name of the event processor
   * @param controlhint The storage control hint to apply for the given event
   * @param purposeString A purpose string which can be used in the skim control
   * configuration
   */
  void addHint(const std::string& processor_name,
               Hint hint,
               const std::string& purposeString);

  /**
   * Add a listening rule
   *
   * These listening rules are regex patterns to decide if a specific hint
   * should be counted when deciding if an event should be kept.
   *
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
   * Collection of hints from the event processors
   *
   * These are only the hints that are from processors matching one
   * of the listening rules.
   */
  std::vector<Hint> hints_;

  /**
   * Collection of rules allowing certain processors
   * or purposes to be considered ("listened to") during
   * the storage decision.
   *
   * Each rule has two entries:
   * 1. a processor regex to match hints coming 
   *    from processors named a certain way
   * 2. a purpose regex to match hints 
   *    from all processors with a specific purpose
   */
  std::vector<std::pair<std::regex, std::regex>> rules_;
};

/**
 * storage control hint alias for backwards compatibility
 */
constexpr StorageControl::Hint hint_shouldKeep = StorageControl::Hint::ShouldKeep;

/**
 * storage control hint alias for backwards compatibility
 */
constexpr StorageControl::Hint hint_shouldDrop = StorageControl::Hint::ShouldDrop;

}  // namespace framework

#endif
