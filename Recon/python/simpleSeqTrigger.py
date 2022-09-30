"""Configuration for TriggerProcessor

Sets all parameters to reasonable defaults.

Attributes:
------------- 
trigger_list
    the collection of each trigger we perform sequential skimming with.

trigger_passNames
    our input pass names for the multiple trigger.

doOR : bool
    skimmer checks if one trigger is true

doAND : bool
    skimmer checks if all triggers are true
doVAL : bool
    skimmer produces an output collection for the purpose of validation

"""

from LDMX.Framework import ldmxcfg

class SequentialTrigger(ldmxcfg.Producer) :
    """Configuration for the sequential trigger (skimmer) on the ECal reco hits"""

    def __init__(self,name) :
        super().__init__(name,'recon::SequentialTrigger','Recon')

        self.trigger_list = ["Trigger","Trigger2"]
        self.trigger_passNames = ["reconSeq","reconSeq"] 
        self.doOR = False
        self.doAND = True
        self.doVAL = True

simpleSeqTrigger = SequentialTrigger("simpleSeqTrigger")

