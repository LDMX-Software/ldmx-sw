from LDMX.Ecal.make_path import make_ci_data_path
from LDMX.Framework import Processor, field, processor


# (Detectors/data/ldmx-det-v15-8gev/constants.gdml)
TS_BAR_COUNT = 24  # number_of_bars
TS_BAR_WIDTH = 3.0  # trigger_bar_dy [mm]
TS_BAR_GAP = 0.3  # trigger_pad_bar_gap [mm]


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

    cluster_coll_name: str = "ecalTrigClusters"
    ele_coll_name: str = "trigElectrons"
    prop_map_name: str = "./propagationMap.root"
    cluster_coll_passname: str = ""
    cluster_collname_events_passname: str = ""
    ts_track_coll_name: str = "TriggerPadTracks"
    ts_passname: str = ""
    # TS channel -> y [mm] = ts_y0 + ts_channel_pitch * centroid:
    # two layers per pad staggered by half a bar, (channel
    # c = 2*bar + layer). ts_y0 (-37.95) is the center of channel 0 and
    # ts_channel_pitch (1.65) is half the bar-to-bar pitch. The real stagger is
    # 1.5 mm, so odd channels sit 0.15 mm below this line
    # (difference <= 2.5 MeV in py).
    ts_y0: float = (
        -(TS_BAR_COUNT * TS_BAR_WIDTH + (TS_BAR_COUNT - 1) * TS_BAR_GAP) / 2
        + TS_BAR_WIDTH / 2
    )
    ts_channel_pitch: float = (TS_BAR_WIDTH + TS_BAR_GAP) / 2


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


@processor("trigger::TrigClusterNNProducer", "Trigger")
class TrigClusterNNProducer(Processor):
    """Configuration for TrigClusterNNProducer

    The ONNX model is kept in the ci-data repository, download it from
    https://github.com/LDMX-Software/ci-data/raw/main/target_genie/ecal_nn_trigger.onnx
    and set model_path to where you saved it.
    """

    model_path: str = field(
        default_factory=lambda: make_ci_data_path("target_genie/ecal_nn_trigger.onnx")
    )
    cluster_coll_name: str = "ecalTrigClusters"
    ecal_sum_coll_name: str = "ecalTrigSums"
    hcal_sum_coll_name: str = "hcalTrigQuadsBackLayerSums"
    input_pass: str = ""
    score_coll_name: str = "ecalTrigClusterNNScore"
