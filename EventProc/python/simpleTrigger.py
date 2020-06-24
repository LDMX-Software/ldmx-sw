"""Configuration for TriggerProcessor

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.EventProc.simpleTrigger import simpleTrigger
    p.sequence.append( simpleTrigger )
"""

from LDMX.Framework import ldmxcfg

class TriggerProcessor(ldmxcfg.Producer) :
    """Configuration for the simple trigger on the ECal"""

    def __init__(self,name) :
        super().__init__(name,'ldmx::TriggerProcessor')

        from LDMX.EventProc import include
        include.library()

        self.threshold = 1500.0
        self.mode = 0
        self.start_layer = 1
        self.end_layer = 20

simpleTrigger = TriggerProcessor("simpleTrigger")

