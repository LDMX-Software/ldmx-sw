from LDMX.Framework.ldmxcfg import Producer

class EcalTPSelector(Producer) :
    """Configuration for EcalTPSelector
    """
    def __init__(self, instance_name = 'myEcalTPSelector') :
        super().__init__(instance_name , 'trigger::EcalTPSelector','Trigger')
        self.tpCollName = "ecalTrigDigis"
        self.passCollName = "ecalTrig"
        
        self.tp_coll_pass_name = ""
        self.tp_coll_event_passname = ""

class TrigEcalEnergySum(Producer) :
    """Configuration for TrigEcalEnergySum
    """
    def __init__(self, instance_name = 'myTrigEcalEnergySum') :
        super().__init__(instance_name , 'trigger::TrigEcalEnergySum','Trigger')
        self.hitCollName = "ecalTrigDigis"
        self.hit_coll_passname = ""
        self.hit_collname_events_passname = ""

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
        self.hit_coll_passname = ""
        self.hit_coll_name_events_passname = ""

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
        
        self.target_sp_passname = ""
        self.cluster_coll_passname = ""
        self.cluster_collname_events_passname = ""
        self.sp_collname_events_passname_ = ""

class NtupleWriter(Producer) :
    """Configuration for Tester
    """
    def __init__(self, instance_name = 'myNtupleWriter', outPath="./ntuple.root", ) :
        super().__init__(instance_name , 'trigger::NtupleWriter','Trigger')
        self.outPath = outPath
        
        self.target_sp_hits_event_passname = ""
        self.target_sp_passname = ""
        self.ecal_sp_hits_events_passname = ""
        self.ecal_sp_passname = ""
        self.ecal_trig_sums_event_passname = ""
        self.ecal_trig_sums_passname = ""
        self.trig_electrons_event_passname = ""
        self.trig_electrons_passname = ""
        self.hcal_trig_quads_events_passname = ""
        self.hcal_trig_quads_passname = ""
        

class PropagationMapWriter(Producer) :
    """Configuration for Tester
    """
    def __init__(self, instance_name = 'myPropagationMapWriter', outPath="./propagationMap.root") :
        super().__init__(instance_name , 'trigger::PropagationMapWriter','Trigger')
        self.outPath = outPath
        
        self.ecal_scoring_plane_passname = ""
        self.target_scoring_plane_passname = ""
        self.target_sp_hits_events_passname = ""
        self.ecal_sp_hits_events_passname = ""
        
