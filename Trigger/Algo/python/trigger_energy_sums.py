from LDMX.Framework.ldmxcfg import Producer


class EcalTPSelector(Producer) :
    """Configuration for EcalTPSelector
    """
    def __init__(self, instance_name = 'myEcalTPSelector') :
        super().__init__(instance_name , 'trigger::EcalTPSelector','Trigger')
        self.tp_coll_name = "ecalTrigDigis"
        self.pass_coll_name = "ecalTrig"
        self.tp_coll_pass_name = ""
        self.tp_coll_event_passname = ""

class HcalTPSelector(Producer) :
    """Configuration for HcalTPSelector (relies on TrigHcalEnergySum)
    """
    def __init__(self, instance_name = 'myHcalTPSelector') :
        super().__init__(instance_name , 'trigger::HcalTPSelector','Trigger')
        self.combined_quad_coll_name = "hcalTrigQuads"
        self.pass_coll_name = "hcalTrig"
        self.tp_coll_pass_name = ""
        self.tp_coll_event_passname = ""

class TrigEcalEnergySum(Producer) :
    """Configuration for TrigEcalEnergySum
    """
    def __init__(self, instance_name = 'myTrigEcalEnergySum') :
        super().__init__(instance_name , 'trigger::TrigEcalEnergySum','Trigger')
        self.hit_coll_name = "ecalTrigDigis"
        self.hit_coll_passname = ""
        self.hit_collname_events_passname = ""

class TrigHcalEnergySum(Producer) :
    """Configuration for TrigHcalEnergySum
    """
    def __init__(self, instance_name = 'myTrigHcalEnergySum') :
        super().__init__(instance_name , 'trigger::TrigHcalEnergySum','Trigger')
        self.quad_coll_name = "hcalTrigPrimDigiSTQs"
        self.combined_quad_coll_name = "hcalTrigQuads"
        self.input_proc = "" # name of the process where the STQs are built

class HCalTrigMipReco(Producer) :
    """Configuration for TrigMipReco in Hcal
    """
    def __init__(self, instance_name = 'myHCalTrigMipReco') :
        super().__init__(instance_name , 'trigger::TrigMipReco','Trigger')
        self.hit_coll_name = "hcalTrigHits"
        self.pass_coll_name = "hcalTrigMIPs"
        self.hit_coll_passname = ""
        self.calorimeter_type_is_hcal = True

class ECalTrigMipReco(Producer) :
    """Configuration for TrigMipReco in Ecal
    """
    def __init__(self, instance_name = 'myECalTrigMipReco') :
        super().__init__(instance_name , 'trigger::TrigMipReco','Trigger')
        self.hit_coll_name = "ecalTrigHits"
        self.pass_coll_name = "ecalTrigMIPs"
        self.hit_coll_passname = ""
        self.calorimeter_type_is_hcal = False

class TrigEcalClusterProducer(Producer) :
    """Configuration for TrigEcalClusterProducer
    """
    def __init__(self, instance_name = 'myTrigEcalClusterProducer') :
        super().__init__(instance_name , 'trigger::TrigEcalClusterProducer','Trigger')
        self.hit_coll_name = "ecalTrigDigis"
        self.cluster_coll_name = "ecalTrigClusters"
        self.hit_coll_passname = ""
        self.hit_coll_name_events_passname = ""

class TrigElectronProducer(Producer) :
    """Configuration for Tester
    """
    def __init__(self, instance_name = 'myTrigElectronProducer', prop_map_name="./propagationMap.root") :
        super().__init__(instance_name , 'trigger::TrigElectronProducer','Trigger')
        # self.out_path = out_path
        self.scoring_plane_coll_name = "TargetScoringPlaneHits"
        self.cluster_coll_name = "ecalTrigClusters"
        self.ele_coll_name = "trigElectrons"
        self.prop_map_name = prop_map_name

        self.target_sp_passname = ""
        self.cluster_coll_passname = ""
        self.cluster_collname_events_passname = ""
        self.sp_collname_events_passname_ = ""

class NtupleWriter(Producer) :
    """Configuration for Tester
    """
    def __init__(self, instance_name = 'myNtupleWriter', out_path="./ntuple.root", ) :
        super().__init__(instance_name , 'trigger::NtupleWriter','Trigger')
        self.out_path = out_path

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
    def __init__(
        self,
        instance_name = 'myPropagationMapWriter',
        out_path = './propagationMap.root'):
        super().__init__(instance_name , 'trigger::PropagationMapWriter','Trigger')
        self.out_path = out_path

        self.ecal_scoring_plane_passname = ""
        self.target_scoring_plane_passname = ""
        self.target_sp_hits_events_passname = ""
        self.ecal_sp_hits_events_passname = ""

