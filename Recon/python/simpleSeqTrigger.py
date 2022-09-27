"""Configuration for TriggerProcessor

Sets all parameters to reasonable defaults.

Attributes:
------------- 
trigger_list
    the collection of each trigger we perform sequential skimming with.

trigger_passName
    our input pass names for the multiple trigger.

pass_mask : int array
    Collection of decimal converted binary masks which induce a pass

seqtrigger_collection : string
    Name of the output collection containing the trigger result

"""

from LDMX.Framework import ldmxcfg

class SequentialTrigger(ldmxcfg.Producer) :
    """Configuration for the (multi-electron aware but simple) sequential trigger on the ECal reco hits"""

    def __init__(self,name) :
        super().__init__(name,'recon::SequentialTrigger','Recon')

        self.trigger_list = ["Trigger","Trigger2"]
        self.trigger_passNames = ["reconSeq","reconSeq"] 
        ANDList = [sum([2**i for i in range(len(self.trigger_list))])]
        self.pass_mask = ANDList #[2,1];# This is OR
        self.seqtrigger_collection = "SeqTrigger"
        self.doOR = False
        self.doAND = True

simpleSeqTrigger = SequentialTrigger("simpleSeqTrigger")

