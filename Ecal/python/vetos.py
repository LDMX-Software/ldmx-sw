from LDMX.Framework import Processor, processor

from .make_path import make_bdt_path, make_roc_path


@processor("ecal::EcalVetoProcessor", "Ecal")
class EcalVetoProcessor(Processor):
    num_ecal_layers: int = 32
    verbose: bool = False
    feature_list_name: str = "input"
    bdt_file: str = make_bdt_path("segmip")
    roc_file: str = make_roc_path("RoC_v14_8gev")
    beam_energy: float = 8000.0  # MeV
    disc_cut: float = 0.99741
    ecal_sp_coll_name: str = "EcalScoringPlaneHits"
    target_sp_coll_name: str = "TargetScoringPlaneHits"
    sp_pass_name: str = ""
    sim_particles_coll_name: str = "SimParticles"
    sim_particles_passname: str = ""
    collection_name: str = "EcalVeto"
    rec_coll_name: str = "EcalRecHits"
    rec_pass_name: str = ""
    recoil_from_tracking: bool = True
    track_collection: str = "RecoilTracksClean"
    track_pass_name: str = ""
    inverse_skim: bool = False


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

    model_path: str = make_bdt_path("particle_net_ecal_v10")
    disc_cut: float = 0.65
    collection_name: str = "EcalPnetVeto"
    rec_coll_name: str = "EcalRecHits"
    ecal_rec_hits_passname: str = ""
    ecal_sp_coll_name: str = "EcalScoringPlaneHits"
    ecal_sp_hits_passname: str = ""
    track_collection: str = "RecoilTracksClean"
    track_pass_name: str = ""
    recoil_from_tracking: bool = True


@processor("ecal::EcalTrackFinderProcessor", "Ecal")
class EcalTrackFinderProcessor(Processor):
    """Configuration for ACTS-based ECAL track finder with zero B-field

    Uses ACTS Combinatorial Kalman Filter to fit straight-line tracks
    through ECAL hits.
    """

    rec_coll_name: str = "EcalRecHits"
    rec_pass_name: str = ""
    out_track_collection: str = "EcalTracks"
    min_hits: int = 3
    max_chi2: float = 10.0
    cell_resolution: float = 1.5  # mm
    debug: bool = False
    max_seed_rms: float = 60.0  # mm
    min_momentum: float = 50.0  # MeV
    max_momentum: float = 10000.0  # MeV
    use_roc_energy: bool = True
    roc_file: str = make_roc_path("RoC_v14_8gev")


@processor("ecal::EcalRecoilRemovalProcessor", "Ecal")
class EcalRecoilRemovalProcessor(Processor):
    """Configuration for the ECal processor which removes hits likely associated
    with the recoil electron."""
    beam_energy: float = 8000.0
    num_ecal_layers: int = 32
    rem_dist_file: str = make_roc_path("RoC_v14_8gev_95")
    collection_name_included: str = "EcalRecHitsInc"
    collection_name_excluded: str = "EcalRecHitsExc"
    rec_coll_name: str = "EcalRecHits"
    rec_pass_name: str = ""
    ecal_sim_pass_name: str = ""
    ecal_sp_hits_pass_name: str = ""
    recoil_from_tracking: bool = True
    track_coll_name: str = "RecoilTracksClean"
    track_pass_name: str = ""
    n_electrons: int = 1

recrem_ecalveto = EcalVetoProcessor(
    bdt_file = make_bdt_path( "2e_wab_vs_signal10mev_v15_1RoC_68" ),
    disc_cut = 0.9913983,
    rec_coll_name = "EcalRecHitsInc",
    collection_name = "EcalVetoInc",
)
recrem_processing = [EcalRecoilRemovalProcessor(), recrem_ecalveto]
