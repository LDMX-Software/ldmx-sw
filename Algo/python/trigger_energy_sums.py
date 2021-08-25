from LDMX.Framework.ldmxcfg import Producer

class TriggerEcalEnergySum(Producer) :
    """Configuration for TriggerEcalEnergySum
    """
    def __init__(self, instance_name = 'myTriggerEcalEnergySum') :
        super().__init__(instance_name , 'trigger::TriggerEcalEnergySum','Trigger')
        self.hitCollName = "ecalTrigDigis"

class TriggerHcalEnergySum(Producer) :
    """Configuration for TriggerHcalEnergySum
    """
    def __init__(self, instance_name = 'myTriggerHcalEnergySum') :
        super().__init__(instance_name , 'trigger::TriggerHcalEnergySum','Trigger')
        self.quadCollName = "hcalOneEndedTrigQuads"
        self.combinedQuadCollName = "hcalTrigQuads"
        
class TrigEcalClusterProducer(Producer) :
    """Configuration for TrigEcalClusterProducer
    """
    def __init__(self, instance_name = 'myTrigEcalClusterProducer') :
        super().__init__(instance_name , 'trigger::TrigEcalClusterProducer','Trigger')
        self.hitCollName = "ecalTrigDigis"
        self.clusterCollName = "ecalTrigClusters"
 
