from LDMX.Framework import Processor, processor


@processor("trigger::EcalTPSelector", "Trigger")
class EcalTPSelector(Processor):
    """Configuration for EcalTPSelector"""

    tp_coll_name: str = "ecalTrigDigis"
    pass_coll_name: str = "ecalTrig"
    tp_coll_pass_name: str = ""
    tp_coll_event_passname: str = ""


@processor("trigger::HcalTPSelector", "Trigger")
class HcalTPSelector(Processor):
    """Configuration for HcalTPSelector (relies on TrigHcalEnergySum)"""

    combined_quad_coll_name: str = "hcalTrigQuads"
    pass_coll_name: str = "hcalTrig"
    tp_coll_pass_name: str = ""
    tp_coll_event_passname: str = ""


@processor("trigger::TrigEcalEnergySum", "Trigger")
class TrigEcalEnergySum(Processor):
    """Configuration for TrigEcalEnergySum"""

    hit_coll_name: str = "ecalTrigDigis"
    hit_coll_passname: str = ""
    hit_collname_events_passname: str = ""


@processor("trigger::TrigHcalEnergySum", "Trigger")
class TrigHcalEnergySum(Processor):
    """Configuration for TrigHcalEnergySum"""

    quad_coll_name: str = "hcalTrigPrimDigiSTQs"
    combined_quad_coll_name: str = "hcalTrigQuads"
    input_proc: str = ""  # name of the process where the STQs are built


@processor("trigger::TrigMipReco", "Trigger")
class HCalTrigMipReco(Processor):
    """Configuration for TrigMipReco in Hcal"""

    hit_coll_name: str = "hcalTrigHits"
    pass_coll_name: str = "hcalTrigMIPs"
    hit_coll_passname: str = ""
    calorimeter_type_is_hcal: bool = True
    max_layer: int = 32
    search_radius: float = 50.0
    min_track_length: int = 5
    isolation_e_cut: float = 180.0
    hole_fraction_max: float = 0.2
    hcal_min_energy: float = 8.0


@processor("trigger::TrigMipReco", "Trigger")
class ECalTrigMipReco(Processor):
    """Configuration for TrigMipReco in Ecal"""

    hit_coll_name: str = "ecalTrigHits"
    pass_coll_name: str = "ecalTrigMIPs"
    hit_coll_passname: str = ""
    calorimeter_type_is_hcal: bool = False
    max_layer: int = 32
    search_radius: float = 50.0
    min_track_length: int = 5
    isolation_e_cut: float = 180.0
    hole_fraction_max: float = 0.2
    ecal_min_energy: float = 3.0
    ecal_max_energy: float = 26.0


@processor("trigger::TrigEcalClusterProducer", "Trigger")
class TrigEcalClusterProducer(Processor):
    """Configuration for TrigEcalClusterProducer"""

    hit_coll_name: str = "ecalTrigDigis"
    cluster_coll_name: str = "ecalTrigClusters"
    hit_coll_passname: str = ""
    hit_coll_name_events_passname: str = ""


@processor("trigger::TrigElectronProducer", "Trigger")
class TrigElectronProducer(Processor):
    """Configuration for TrigElectronProducer"""

    scoring_plane_coll_name: str = "TargetScoringPlaneHits"
    cluster_coll_name: str = "ecalTrigClusters"
    ele_coll_name: str = "trigElectrons"
    prop_map_name: str = "./propagationMap.root"
    target_sp_passname: str = ""
    cluster_coll_passname: str = ""
    cluster_collname_events_passname: str = ""
    sp_collname_events_passname_: str = ""


@processor("trigger::NtupleWriter", "Trigger")
class NtupleWriter(Processor):
    """Configuration for NtupleWriter"""

    out_path: str = "./ntuple.root"
    target_sp_hits_event_passname: str = ""
    target_sp_passname: str = ""
    ecal_sp_hits_events_passname: str = ""
    ecal_sp_passname: str = ""
    ecal_trig_sums_event_passname: str = ""
    ecal_trig_sums_passname: str = ""
    trig_electrons_event_passname: str = ""
    trig_electrons_passname: str = ""
    hcal_trig_quads_events_passname: str = ""
    hcal_trig_quads_passname: str = ""


@processor("trigger::PropagationMapWriter", "Trigger")
class PropagationMapWriter(Processor):
    """Configuration for PropagationMapWriter"""

    out_path: str = "./propagationMap.root"
    ecal_scoring_plane_passname: str = ""
    target_scoring_plane_passname: str = ""
    target_sp_hits_events_passname: str = ""
    ecal_sp_hits_events_passname: str = ""

