"""Configuration for TriggerProcessor

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.Recon.simpleTrigger import simpleTrigger
    p.sequence.append( simpleTrigger )
"""

from LDMX.Framework import ldmxcfg

class TriggerProcessor(ldmxcfg.Producer) :
    """Configuration for the simple trigger on the ECal"""

    def __init__(self,name) :
        super().__init__(name,'recon::TriggerProcessor','Recon')

        self.threshold = 1500.0
        self.mode = 0
        self.start_layer = 1
        self.end_layer = 20
        self.input_collection = "EcalRecHits"
        self.trigger_collection = "Trigger"

simpleTrigger = TriggerProcessor("simpleTrigger")

