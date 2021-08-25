from LDMX.Framework.ldmxcfg import Producer

class EcalTPSelector(Producer) :
    """Configuration for EcalTPSelector
    """
    def __init__(self, instance_name = 'myEcalTPSelector') :
        super().__init__(instance_name , 'trigger::EcalTPSelector','Trigger')
        self.tpCollName = "ecalTrigDigis"
        self.passCollName = "ecalTrig"

class TrigEcalEnergySum(Producer) :
    """Configuration for TrigEcalEnergySum
    """
    def __init__(self, instance_name = 'myTrigEcalEnergySum') :
        super().__init__(instance_name , 'trigger::TrigEcalEnergySum','Trigger')
        self.hitCollName = "ecalTrigDigis"

class TrigHcalEnergySum(Producer) :
    """Configuration for TrigHcalEnergySum
    """
    def __init__(self, instance_name = 'myTrigHcalEnergySum') :
        super().__init__(instance_name , 'trigger::TrigHcalEnergySum','Trigger')
        self.quadCollName = "hcalOneEndedTrigQuads"
        self.combinedQuadCollName = "hcalTrigQuads"

class TrigEcalClusterProducer(Producer) :
    """Configuration for TrigEcalClusterProducer
    """
    def __init__(self, instance_name = 'myTrigEcalClusterProducer') :
        super().__init__(instance_name , 'trigger::TrigEcalClusterProducer','Trigger')
        self.hitCollName = "ecalTrigDigis"
        self.clusterCollName = "ecalTrigClusters"
 
