#include "Framework/StorageControl.h"

#include "Framework/Exception/Exception.h"

namespace framework {

void StorageControl::resetEventState() { hints_.clear(); }

void StorageControl::addHint(const std::string& processor_name, Hint hint,
                             const std::string& purposeString) {
  for (const auto& [processor_rule, purpose_rule] : rules_) {
    if (std::regex_match(processor_name, processor_rule) and
        std::regex_match(purposeString, purpose_rule)) {
      // cache hints that matched a rule for later tallying
      hints_.push_back(hint);
      // leave after first match to avoid double-counting
      break;
    }
  }
}

void StorageControl::addRule(const std::string& processor_pat,
                             const std::string& purpose_pat) {
  /**
   * @note Rules that don't specify a processor pattern are ignored.
   */
  if (processor_pat.empty()) return;

  try {
    rules_.emplace_back(
        std::piecewise_construct,
        std::forward_as_tuple(processor_pat,
                              std::regex::extended | std::regex::nosubs),
        std::forward_as_tuple(purpose_pat.empty() ? ".*" : purpose_pat,
                              std::regex::extended | std::regex::nosubs));
  } catch (const std::regex_error& e) {
    // re-throw the regex error with our error
    std::string msg{
        "Invalid regex configured for the storage control listening rules: "};
    msg += e.what();
    EXCEPTION_RAISE("ConfigureError", msg);
  }
}

bool StorageControl::keepEvent(bool event_completed) const {
  /**
   * @note If event_completed is false, we don't listen to **anything** and
   * decide not to keep the event.
   */
  if (not event_completed) return false;

  /**
   * loop over the hints provided by processors we are listening to.
   */
  int votes_keep(0), votes_drop(0);
  bool must_drop{false}, must_keep{false};
  for (auto hint : hints_) {
    switch (hint) {
      case Hint::MustDrop:
        must_drop = true;
        break;
      case Hint::MustKeep:
        must_keep = true;
        break;
      case Hint::ShouldDrop:
        votes_drop++;
        break;
      case Hint::ShouldKeep:
        votes_keep++;
        break;
      case Hint::Undefined:
      case Hint::NoOpinion:
        break;
      default:
        // how did I get here?
        EXCEPTION_RAISE(
            "SupaBad",
            "This error comes from StorageControl and should never happen. "
            "A storage hint should always be one of the members of the "
            "StorageControlHint enum.");
    }
  }

  /**
   * mustDrop is highest priority, if it exists the event is dropped
   */
  if (must_drop) return false;

  /**
   * mustKeep is second highest, if it exists when mustDrop does not, the event
   * is kept
   */
  if (must_keep) return true;

  /**
   * If we don't have any 'must' hints, we tally votes
   * and follow the choice made by a simple majority.
   */
  if (votes_keep > votes_drop) return true;
  if (votes_drop > votes_keep) return false;

  /**
   * If there is a tie in the vote (including the case
   * where there were no votes), then we use the default
   * decision.
   */
  return defaultIsKeep_;
}
}  // namespace framework
