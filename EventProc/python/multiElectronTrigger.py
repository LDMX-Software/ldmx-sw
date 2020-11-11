"""Configuration for MultiElectronTriggerProcessor

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.EventProc.multiElectronTrigger import multiElectronTrigger
    p.sequence.append( multiElectronTrigger )
"""

from LDMX.Framework import ldmxcfg

class MultiElectronTriggerProcessor(ldmxcfg.Producer) :
    """Configuration for the multiElectron trigger on the ECal"""

    def __init__(self,name) :
        super().__init__(name,'ldmx::MultiElectronTriggerProcessor','EventProc')

        self.beamEnergy = 4000.
        self.thresholds = [ 1500.0, 1000. + self.beamEnergy, 500. + 2*self.beamEnergy, 100. + 3*self.beamEnergy ]  #toy something up 
        self.mode = 0
        self.start_layer = 1
        self.end_layer = 20
        self.input_collection = "EcalRecHits"
        self.trigger_collection = "Trigger"

multiElectronTrigger = MultiElectronTriggerProcessor("multiElectronTrigger")

