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
        self.quadCollName = "hcalTrigPrimDigiSTQs"
        self.combinedQuadCollName = "hcalTrigQuads"
        self.inputProc = "" # name of the process where the STQs are built

class TrigEcalClusterProducer(Producer) :
    """Configuration for TrigEcalClusterProducer
    """
    def __init__(self, instance_name = 'myTrigEcalClusterProducer') :
        super().__init__(instance_name , 'trigger::TrigEcalClusterProducer','Trigger')
        self.hitCollName = "ecalTrigDigis"
        self.clusterCollName = "ecalTrigClusters"

class TrigElectronProducer(Producer) :
    """Configuration for Tester
    """
    def __init__(self, instance_name = 'myTrigElectronProducer', propMapName="./propagationMap.root") :
        super().__init__(instance_name , 'trigger::TrigElectronProducer','Trigger')
        # self.outPath = outPath
        self.scoringPlaneCollName = "TargetScoringPlaneHits"
        self.clusterCollName = "ecalTrigClusters"
        self.eleCollName = "trigElectrons"
        self.propMapName = propMapName

class NtupleWriter(Producer) :
    """Configuration for Tester
    """
    def __init__(self, instance_name = 'myNtupleWriter', outPath="./ntuple.root", ) :
        super().__init__(instance_name , 'trigger::NtupleWriter','Trigger')
        self.outPath = outPath

class PropagationMapWriter(Producer) :
    """Configuration for Tester
    """
    def __init__(self, instance_name = 'myPropagationMapWriter', outPath="./propagationMap.root") :
        super().__init__(instance_name , 'trigger::PropagationMapWriter','Trigger')
        self.outPath = outPath
