"""Configuration for Hcal pipeline"""

from LDMX.Framework import Processor, processor


@processor("hcal::HcalVetoProcessor", "Hcal")
class HcalVetoProcessor(Processor):
    pe_threshold: float = 8.0
    max_time: float = 50.0
    back_min_pe: float = 1.0
    input_hit_coll_name: str = "HcalRecHits"
    input_hit_pass_name: str = ""
    output_coll_name: str = "HcalVeto"
    track_pass_name: str = ""
    inverse_skim: bool = False


@processor("hcal::HcalWABVetoProcessor", "Hcal")
class HcalWABVetoProcessor(Processor):
    max_total_energy_compare: float = 1000.0
    min_total_energy_compare: float = 0.0
    n_clusters: float = 6.0
    mean_hits_per_cluster: float = 3.0
    mean_energy_per_cluster: float = 6.0
    input_hcal_hit_coll_name: str = "HcalRecHits"
    input_ecal_hit_coll_name: str = "EcalRecHits"
    output_coll_name: str = "HcalWABVetoes"
    input_hcal_cluster_coll_name: str = "HcalClusters"
    hcal_hit_passname: str = ""
    ecal_hit_passname: str = ""
    hcal_cluster_passname: str = ""
    track_pass_name: str = ""
    track_pass_name: str = ""


@processor("hcal::HcalOldDigiProducer", "Hcal")
class HcalOldDigiProducer(Processor):
    meanNoise: float = 0.02
    readoutThreshold: int = 1
    strips_side_lr_per_layer: int = 12
    num_side_lr_hcal_layers: int = 26
    strips_side_tb_per_layer: int = 12
    num_side_tb_hcal_layers: int = 28
    strips_back_per_layer: int = 60  # n strips correspond to 5 cm wide bars
    num_back_hcal_layers: int = 96
    super_strip_size: int = 1  # 1 = 5 cm readout, 2 = 10 cm readout, ...
    mev_per_mip: float = (
        4.66  # measured 1.4 MeV for a 6mm thick tile, so for 20mm bar = 1.4*20/6
    )
    pe_per_mip: float = 68.0  # PEs per MIP at 1m (assume 80% attentuation of 1m)
    strip_attenuation_length: float = 5.0  # this is in m
    strip_position_resolution: float = 150.0  # this is in mm
    sim_hit_pass_name: str = ""  # use any pass available


@processor("hcal::HcalClusterProducer", "Hcal")
class HcalClusterProducer(Processor):
    # EminSeed: float = 0.1 # Not used
    enoise_cut: float = 0.01
    delta_time: float = 10.0
    delta_r: float = 0.0
    emin_cluster: float = 0.5  # Minimum Energy to be classed as a cluster TODO
    cut_off: float = 10.0
    cluster_coll_name: str = "HcalClusters"
    hcal_hits_pass_name: str = ""
