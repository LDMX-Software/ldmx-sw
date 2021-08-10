
from LDMX.Framework.ldmxcfg import Producer

class TriggerEcalEnergySum(Producer) :
    """Configuration for TriggerEcalEnergySum
    """

    def __init__(self, instance_name = 'myTriggerEcalEnergySum') :
        super().__init__(instance_name , 'trigger::TriggerEcalEnergySum','Trigger')
