#include "Framework/StorageControl.h"
#include <regex.h>
#include <algorithm>
#include <sys/types.h>
#include "Framework/Exception/Exception.h"

namespace framework {

void StorageControl::resetEventState() { hints_.clear(); }

void StorageControl::addHint(const std::string& processor_name,
                             framework::StorageControlHint hint,
                             const std::string& purposeString) {
  hints_.push_back(Hint());
  hints_.back().evpName_ = processor_name;
  hints_.back().hint_ = hint;
  hints_.back().purpose_ = purposeString;
}

void StorageControl::addRule(const std::string& processor_pat,
                             const std::string& purpose_pat) {
  if (processor_pat.empty()) return;

  regex_t* preg = new regex_t;
  int error = regcomp(preg, processor_pat.c_str(), REG_EXTENDED | REG_NOSUB);
  if (error) {
    char msg[1024];
    regerror(error, preg, msg, 1024);
    delete preg;
    EXCEPTION_RAISE("SkimRuleException", msg);
  }
  // ok, then keep it
  rules_.push_back(Rule());
  rules_.back().evpNameRegex_ = preg;

  if (!purpose_pat.empty()) {
    preg = new regex_t;
    int error = regcomp(preg, purpose_pat.c_str(), REG_EXTENDED | REG_NOSUB);
    if (error) {
      char msg[1024];
      regerror(error, preg, msg, 1024);
      delete preg;
      EXCEPTION_RAISE("SkimRuleException", msg);
    }
    rules_.back().purposeRegex_ = preg;
  }

  rules_.back().evpNamePattern_ = processor_pat;
  rules_.back().purposePattern_ = purpose_pat;
}

bool StorageControl::Rule::matches(const StorageControl::Hint& h) const {
  if (regexec((const regex_t*)(evpNameRegex_), h.evpName_.c_str(), 0, 0, 0))
    return false;
  if (purposeRegex_ != 0 &&
      regexec((const regex_t*)(purposeRegex_), h.purpose_.c_str(), 0, 0, 0))
    return false;
  return true;
}

bool StorageControl::keepEvent(bool event_completed) const {
  /**
   * @note If event_completed is false, we don't listen to **anything** and
   * decide not to keep the event.
   */
  if (not event_completed) return false;

  /**
   * loop over the hints provided by processors,
   * if a hint matches any of the rules configured,
   * we will listen to it.
   */
  int votesKeep(0), votesDrop(0);
  bool mustDrop{false}, mustKeep{false};
  for (auto hint : hints_) {
    if (std::any_of(rules_.begin(), rules_.end(), [&hint](const Rule& r) { return r.matches(hint); })) {
      switch(hint.hint_) {
        case hint_mustDrop:
          mustDrop = true;
          break;
        case hint_mustKeep:
          mustKeep = true;
          break;
        case hint_shouldDrop:
          votesDrop++;
          break;
        case hint_shouldKeep:
          votesKeep++;
          break;
        case hint_Undefined:
        case hint_NoOpinion:
          break;
        default:
          // how did I get here?
          EXCEPTION_RAISE(
              "SupaBad",
              "This error comes from StorageControl and should never happen. "
              "A storage hint should always be one of the members of the StorageControlHint enum."
          );
      }
    }
  }

  /**
   * mustDrop is highest priority, if it exists the event is dropped
   */
  if (mustDrop) return false;

  /**
   * mustKeep is second highest, if it exists when mustDrop does not, the event is kept
   */
  if (mustKeep) return true;

  /**
   * If we don't have any 'must' hints, we tally votes
   * and follow the choice made by a simple majority.
   */
  if (votesKeep > votesDrop) return true;
  if (votesDrop > votesKeep) return false;

  /**
   * If there is a tie in the vote (including the case
   * where there were no votes), then we use the default
   * decision.
   */
  return defaultIsKeep_;
}
}  // namespace framework
