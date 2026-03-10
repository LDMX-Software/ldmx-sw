from LDMX.Framework import Processor, processor
from .make_path import makeBDTPath, makeRoCPath


@processor("ecal::EcalVetoProcessor", "Ecal")
class EcalVetoProcessor(Processor):
    num_ecal_layers: int = 32
    verbose: bool = False
    feature_list_name: str = "input"
    bdt_file: str = makeBDTPath("segmip")
    roc_file: str = makeRoCPath("ROC_v14_8gev")
    beam_energy: float = 8000.0  # MeV
    disc_cut: float = 0.99741
    sp_pass_name: str = ""
    collection_name: str = "EcalVeto"
    rec_pass_name: str = ""
    rec_coll_name: str = "EcalRecHits"
    recoil_from_tracking = True
    track_collection: str = "RecoilTracksClean"
    inverse_skim: bool = False
    sim_particles_passname: str = ""
    track_pass_name: str = ""
    ecal_simhits_passname: str = ""
    ecal_digis_passname: str = ""
    ecal_rechits_passname: str = ""
    ecal_trig_digis_passname: str = ""


@processor("ecal::EcalMipTrackingProcessor", "Ecal")
class EcalMipProcessor(Processor):
    num_ecal_layers: int = 32
    linreg_radius: float = 35.0  # in mm
    ecal_collection_name: str = "EcalVeto"
    ecal_pass_name: str = ""
    mip_collection_name: str = "EcalTrajectoryInfo"
    mip_pass_name: str = ""
    mip_result_name: str = "EcalMipInfo"


@processor("ecal::EcalPnetVetoProcessor", "Ecal")
class EcalPnetVetoProcessor(Processor):
    """Configuration for ParticleNet Ecal Veto

    ParticleNet trained on v14 geometry ecalPN + signal
    """

    model_path: str = makeBDTPath("particle_net_ecal_v10")
    disc_cut: float = 0.65
    collection_name: str = "EcalPnetVeto"
    rec_coll_name: str = "EcalRecHits"
    ecal_rec_hits_passname: str = ""
    ecal_sp_hits_passname: str = ""
    track_collection: str = "RecoilTracksClean"
    track_pass_name: str = ""
    recoil_from_tracking: bool = True
