"""Configuration for TriggerProcessor

Sets all parameters to reasonable defaults.

Attributes:
------------- 
trigger_list
    the collection of each trigger we perform sequential skimming with.

trigger_passName
    our input pass names for the multiple triggers. Should be the same for all

pass_mask : int array
    Collection of decimal converted binary masks which induce a pass

seqtrigger_collection : string
    Name of the output collection containing the trigger result


Examples
--------
    from LDMX.Recon.simpleTrigger import simpleTrigger
    p.sequence.append( simpleTrigger )
"""

from LDMX.Framework import ldmxcfg

class SequentialTrigger(ldmxcfg.Producer) :
    """Configuration for the (multi-electron aware but simple) sequential trigger on the ECal reco hits"""

    def __init__(self,name) :
        super().__init__(name,'recon::SequentialTrigger','Recon')

        self.trigger_list = ["trigger1","trigger2"]
        self.trigger_passName = "pass_"
        self.pass_mask = [2,1];# This is OR
        self.seqtrigger_collection = "SeqTrigger"

simpleSeqTrigger = SequentialTrigger("simpleSeqTrigger")

