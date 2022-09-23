
#include "Recon/SequentialTrigger.h"
#include "Recon/Event/TriggerResult.h"

namespace recon {

void SequentialTrigger::configure(framework::config::Parameters &ps) {
  trigger_list_ = ps.getParameter<std::vector<std::string>>("trigger_list");
  trigger_passNames_ = ps.getParameter<std::vector<std::string>>("trigger_passNames");
  pass_masks_ = ps.getParameter<std::vector<int>>("pass_mask");
  outputColl_ = ps.getParameter<std::string>("seqtrigger_collection");
  return;
}

void SequentialTrigger::produce(framework::Event& event) {
  /** Grab the Ecal hit collection for the given event */
  bool hasPassed = false;
  int  maskValue = 0; 
  for (int i = 0; i<trigger_list_.size(); i++){
    auto trigResult{event.getObject<ldmx::TriggerResult>(trigger_list_[i],
                                                  trigger_passNames_[i])};
    if(trigResult.passed()){
        maskValue+=(int)pow(2,i);
    }  
  }  
  for(int m : pass_masks_){
    if(maskValue==m){hasPassed=true;}
  }
  event.add(outputColl_, hasPassed);
  // mark the event
  if (hasPassed)
    setStorageHint(framework::hint_shouldKeep);
  else
    setStorageHint(framework::hint_shouldDrop);
}

}  // namespace dqm

DECLARE_ANALYZER_NS(recon,SequentialTrigger);
