#include "Recon/SequentialTrigger.h"
#include "Recon/Event/TriggerResult.h"

namespace recon {

/**
 *Instantiates the variables used in this processor. We take trigger_list_ and tigger_passNames_ to obtain trigger collections, a doOR_ and doAND_ option to check if one of or all events pass, and doVAL_ to output a validation collection
 * */

void SequentialTrigger::configure(framework::config::Parameters &ps) {
  trigger_list_ = ps.getParameter<std::vector<std::string>>("trigger_list");
  trigger_passNames_ = ps.getParameter<std::vector<std::string>>("trigger_passNames");
  doAND_ = ps.getParameter<bool>("doAND");
  doOR_ = ps.getParameter<bool>("doOR");
  doVAL_ = ps.getParameter<bool>("doVAL");
  return;
}

/**
 *
 *This producer takes in a list of triggers and runs an OR or AND skimmer depending on the config file. It will change the keep
 tag and produce nothing. This is unless the validation command is set to true, in which case it produces a boolean collection in an output file
 with the keeping flag.
 *
 * */

void SequentialTrigger::produce(framework::Event& event) {
  bool hasPassed = false;
  bool keepTrack = false;
  //Returns an error if some bad combination of doOR and doAND is enabled.
  if (doAND_ and doOR_){
    ldmx_log(fatal) << "Can't do both doAND and doOR. Exiting.";
    return;
  }
  if (not(doAND_) and not(doOR_)){
    ldmx_log(fatal) << "Must do one of doAND and doOR. Exiting.";
    return;
  }
  for (int i = 0; i<trigger_list_.size(); i++){
    //Returns an error is a trigger collection DNE
    if (!event.exists(trigger_list_[i],trigger_passNames_[i])) {
      ldmx_log(fatal) << "Attemping to use non-existing trigger collection "
                      << trigger_list_[i] << "_" << trigger_passNames_[i]
                      << " to skim! Exiting.";
      return;
    }
    auto trigResult{event.getObject<ldmx::TriggerResult>(trigger_list_[i],trigger_passNames_[i])};
    //Returns true should any trigger pass and doOR_ enabled, and sets keepTrack to true and breaks if doAND_ and any fail
    if(trigResult.passed()){
        if(doOR_){hasPassed=true;break;}
    }else{
        if(doAND_){keepTrack=true;break;}
    }
  }
  //In the event we have doAND_ set, this will run and only return true if all are true
  if(doAND_){hasPassed=not(keepTrack);}
  
  //Used to validate if code was working
  if(doVAL_){
  event.add("validation", hasPassed);
  }
  // mark the event
  if (hasPassed)
    setStorageHint(framework::hint_shouldKeep);
  else
    setStorageHint(framework::hint_shouldDrop);
}

}

DECLARE_ANALYZER_NS(recon,SequentialTrigger);
