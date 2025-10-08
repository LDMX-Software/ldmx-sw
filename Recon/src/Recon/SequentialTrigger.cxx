#include "Recon/SequentialTrigger.h"

#include "Recon/Event/TriggerResult.h"

namespace recon {

/**
 *Instantiates the variables used in this processor. We take trigger_list_ and
 *trigger_passNames_ to obtain trigger collections, a do_or_ and do_and_ option
 *to check if one of or all events pass, and do_val_ to output a validation
 *collection
 * */

void SequentialTrigger::configure(framework::config::Parameters& ps) {
  trigger_list_ = ps.get<std::vector<std::string>>("trigger_list");
  trigger_pass_names_ = ps.get<std::vector<std::string>>("trigger_passNames");
  do_and_ = ps.get<bool>("doAND");
  do_or_ = ps.get<bool>("doOR");
  do_val_ = ps.get<bool>("doVAL");
  // Returns an error if some bad combination of doOR and doAND is enabled.
  if (do_and_ == do_or_) {
    EXCEPTION_RAISE("InvalidArg",
                    "Either tried to do both or neither of doAND and doOR. "
                    "Exactly one should be true.");
  }
  return;
}

/**
 *
 *This producer takes in a list of triggers and runs an OR or AND skimmer
 depending on the config file. It will change the keep
 tag and produce nothing. This is unless the validation command is set to true,
 in which case it produces a boolean collection in an output file
 with the keeping flag.
 *
 * */

void SequentialTrigger::produce(framework::Event& event) {
  bool has_passed = not(do_or_) or (do_and_);

  for (int i = 0; i < trigger_list_.size(); i++) {
    // Returns an error is a trigger collection DNE
    try {
      auto trig_result{event.getObject<ldmx::TriggerResult>(
          trigger_list_[i], trigger_pass_names_[i])};

      // Returns true should any trigger pass and do_or_ enabled, and returns
      // false if do_and_ and any fail
      if (trig_result.passed()) {
        if (do_or_) {
          has_passed = true;
          break;
        }
      } else {
        if (do_and_) {
          has_passed = false;
          break;
        }
      }
    } catch (...) {
      std::string error_message =
          "Attemping to use non-existing trigger collection " +
          trigger_list_[i] + "_" + trigger_pass_names_[i] +
          " to skim! Exiting.";
      EXCEPTION_RAISE("InvalidArg", error_message.data());
      return;
    }
  }

  // Used to validate if code was working
  if (do_val_) {
    event.add("validation", has_passed);
  }
  // mark the event
  if (has_passed)
    setStorageHint(framework::HINT_SHOULD_KEEP);
  else
    setStorageHint(framework::HINT_SHOULD_DROP);
}
}  // namespace recon

DECLARE_ANALYZER(recon::SequentialTrigger);
