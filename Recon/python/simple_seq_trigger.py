"""Configuration for TriggerProcessor

Sets all parameters to reasonable defaults.

Attributes:
-------------
trigger_list
    the collection of each trigger we perform sequential skimming with.

trigger_pass_names
    our input pass names for the multiple trigger.

doOR : bool
    skimmer checks if one trigger is true

doAND : bool
    skimmer checks if all triggers are true
doVAL : bool
    skimmer produces an output collection for the purpose of validation

"""

from LDMX.Framework import Processor, processor


@processor("recon::SequentialTrigger", "Recon")
class SequentialTrigger(Processor):
    """Configuration for the sequential trigger (skimmer) on the ECal reco hits"""

    trigger_list: list[str] = ["Trigger", "Trigger2"]
    trigger_pass_names: list[str] = ["reconSeq", "reconSeq"]
    do_or: bool = False
    do_and: bool = True
    do_val: bool = True


simple_seq_trigger = SequentialTrigger(instance_name="simple_seq_trigger")
