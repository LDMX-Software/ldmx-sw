
from LDMX.Framework.ldmxcfg import Producer

class TrigEcalEnergySum(Producer) :
    """Configuration for TrigEcalEnergySum
    """

    def __init__(self, instance_name = 'myTrigEcalEnergySum') :
        super().__init__(instance_name , 'trigger::TrigEcalEnergySum','Trigger')
