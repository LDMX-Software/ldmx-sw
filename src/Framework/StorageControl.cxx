#include "Framework/StorageControl.h"
#include <regex.h>
#include <sys/types.h>
#include "Framework/Exception/Exception.h"

namespace ldmx {

void StorageControl::resetEventState() { hints_.clear(); }

void StorageControl::addHint(const std::string& processor_name,
                             ldmx::StorageControlHint hint,
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

bool StorageControl::Rule::matches(const StorageControl::Hint& h) {
  if (regexec((const regex_t*)(evpNameRegex_), h.evpName_.c_str(), 0, 0, 0))
    return false;
  if (purposeRegex_ != 0 &&
      regexec((const regex_t*)(purposeRegex_), h.purpose_.c_str(), 0, 0, 0))
    return false;
  return true;
}

bool StorageControl::keepEvent() const {
  int votesKeep(0), votesDrop(0);
  // loop over all rules and then over all hints
  for (auto rule : rules_) {
    for (auto hint : hints_) {
      if (!rule.matches(hint)) continue;
      if (hint.hint_ == hint_shouldKeep || hint.hint_ == hint_mustKeep)
        votesKeep++;
      else if (hint.hint_ == hint_shouldDrop || hint.hint_ == hint_mustDrop)
        votesDrop++;
    }
  }

  // easy case
  if (!votesKeep && !votesDrop) return defaultIsKeep_;

  // harder cases
  if (votesKeep > votesDrop) return true;
  if (votesDrop > votesKeep) return false;

  // at the end, go with the default
  return defaultIsKeep_;
}
}  // namespace ldmx
