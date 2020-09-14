
from LDMX.Framework import ldmxcfg
from LDMX.DetDescr import EcalHexReadout

class EcalGeometryProvider(ldmxcfg.ConditionsObjectProvider):
    def __init__(self,tagName):
        super().__init__("EcalGeometryProvider","ldmx::EcalGeometryProvider",tagName)
        self.EcalHexReadout = EcalHexReadout.EcalHexReadout()
        
