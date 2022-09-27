
#include "Recon/SequentialTrigger.h"
#include "Recon/Event/TriggerResult.h"

namespace recon {

void SequentialTrigger::configure(framework::config::Parameters &ps) {
  trigger_list_ = ps.getParameter<std::vector<std::string>>("trigger_list");
  trigger_passNames_ = ps.getParameter<std::vector<std::string>>("trigger_passNames");
  pass_masks_ = ps.getParameter<std::vector<int>>("pass_mask");
  doAND_ = ps.getParameter<bool>("doAND");
  doOR_ = ps.getParameter<bool>("doOR");
  return;
}

void SequentialTrigger::produce(framework::Event& event) {
  bool hasPassed = false;
  bool keepTrack = false;
  int  maskValue = 0; 
  for (int i = 0; i<trigger_list_.size(); i++){
    //if (!event.exists(trigger_list_[i],trigger_passNames_[i])) {
    //  ldmx_log(fatal) << "Attemping to use non-existing trigger collection "
    //                  << trigger_list_[i] << "_" << trigger_passNames_[i]
    //                  << " to skim! Exiting.";
    //  return;
    //}
    if (doAND_ and doOR_){
      ldmx_log(fatal) << "Can't do both doAND and doOR. Exiting.";
      return;
    }
    auto trigResult{event.getObject<ldmx::TriggerResult>(trigger_list_[i],trigger_passNames_[i])};
    //Returns true should any trigger pass and doOR_ enabled; otherwise preps to use masks
    if(trigResult.passed()){
        if(doOR_){hasPassed=true;break;}
        maskValue+=(int)pow(2,i);
    }else{
        keepTrack=true;
    }
  }
  //In the event we have doAND_ set, this will run and only return true if all are true
  if(doAND_){hasPassed=not(keepTrack);}
  //Uses masks should other methods fail
  if(not(doOR_)and not(doAND_)){
    for(int m : pass_masks_){
        if(maskValue==m){hasPassed=true;break;}
    }
  }
  //Used to validate if code was working

  event.add("validation", hasPassed);
  
  // mark the event
  if (hasPassed)
    setStorageHint(framework::hint_shouldKeep);
  else
    setStorageHint(framework::hint_shouldDrop);
}

}

DECLARE_ANALYZER_NS(recon,SequentialTrigger);
