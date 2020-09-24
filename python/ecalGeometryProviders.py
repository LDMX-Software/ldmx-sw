"""Configuration of the EcalHexReadout and EcalTriggerGeometry

"""

from LDMX.Framework.ldmxcfg import ConditionsObjectProvider
from LDMX.DetDescr import EcalHexReadout

class EcalGeometryProvider(ConditionsObjectProvider) :
    """Configuration for EcalGeometryProvider, which is the source of EcalHexReadout
       and EcalTriggerGeometry.
    """
    
    def __init__(self, tagName):
        super().__init__("EcalGeometryProvider","ldmx::EcalGeometryProvider",tagName)
        self.provides=[]
        self.hexReadout = EcalHexReadout.EcalHexReadout()


