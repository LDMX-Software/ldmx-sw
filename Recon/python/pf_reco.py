"""Configuration for pfReco producers

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.Recon.pf_reco import PFEcalClusterProducer
    p.sequence.append( PFEcalClusterProducer )
"""

from LDMX.Framework import Processor, processor


@processor("recon::PFEcalClusterProducer", "Recon")
class PFEcalClusterProducer(Processor):
    """Configuration for Ecal cluster builder for particle reco"""

    hit_coll_name: str = "EcalRecHits"
    hit_pass_name: str = ""
    cluster_coll_name: str = "PFEcalClusters"
    do_single_cluster: bool = False
    log_energy_weight: bool = True
    min_cluster_hit_mult: int = 2
    cluster_hit_dist: float = 50.0
    cluster_z_bias: float = 1.0
    min_hit_energy: float = 1.0
    save_hit_contribs: bool = True


@processor("recon::PFHcalClusterProducer", "Recon")
class PFHcalClusterProducer(Processor):
    """Configuration for Hcal cluster builder for particle reco"""

    hit_coll_name: str = "HcalRecHits"
    hit_pass_name: str = ""
    cluster_coll_name: str = "PFHcalClusters"
    do_single_cluster: bool = False
    log_energy_weight: bool = True
    min_cluster_hit_mult: int = 2
    cluster_hit_dist: float = 100.0
    cluster_z_bias: float = 1.0
    min_hit_energy: float = 0.1
    save_hit_contribs: bool = True


@processor("recon::PFTrackProducer", "Recon")
class PFTrackProducer(Processor):
    """Configuration for track selector for particle reco

    Outputs ldmx::Track. With truth_tracking, Ecal scoring plane hits are
    wrapped into tracks; otherwise reconstructed tracks with an AtECAL
    state are selected.
    """

    truth_tracking: bool = False
    # truth mode
    input_track_coll_name: str = "EcalScoringPlaneHits"
    input_pass_name: str = ""
    do_electron_tracking: bool = False
    min_electron_momentum_z: float = 2500.0
    max_electron_track_id: int = 30
    # reco mode
    reco_track_coll_name: str = "RecoilTracksClean"
    reco_track_pass_name: str = ""
    min_n_hits: int = 0
    max_chi2_ndf: float = -1.0  # negative disables
    min_momentum: float = 0.0  # MeV at Ecal face
    output_track_coll_name: str = "PFTracks"


@processor("recon::ParticleFlow", "Recon")
class PFProducer(Processor):
    """Configuration for particle reco"""

    input_ecal_coll_name: str = "PFEcalClusters"
    input_hcal_coll_name: str = "PFHcalClusters"
    input_track_coll_name: str = "PFTracks"
    use_existing_ecal_clusters: bool = False
    output_coll_name: str = "PFCandidates"
    single_particle: bool = False
    input_ecal_passname: str = ""
    input_hcal_passname: str = ""
    input_tracks_passname: str = ""
    truth_sp_coll_name: str = "EcalScoringPlaneHits"
    truth_sp_passname: str = ""
    # energy corrections vs raw cluster energy [MeV], empty means none
    # old Ecal table overcorrected electrons, disabled until rederived
    ecal_corr_energies: list[float] = []
    ecal_corr_factors: list[float] = []
    hcal_corr_energies: list[float] = [
        25.0,
        75.0,
        125.0,
        175.0,
        225.0,
        275.0,
        325.0,
        375.0,
        425.0,
    ]
    hcal_corr_factors: list[float] = [
        8.44,
        7.38,
        7.76,
        8.535,
        9.47,
        10.45,
        10.47,
        9.71,
        8.87,
    ]


@processor("recon::PFTruthProducer", "Recon")
class PFTruthProducer(Processor):
    """Configuration for track selector for particle reco"""

    output_primary_coll_name: str = "PFTruth"
    output_target_coll_name: str = "PFTruthTarget"
    output_ecal_coll_name: str = "PFTruthEcal"
    output_hcal_coll_name: str = "PFTruthHcal"
    target_sp_coll_name: str = "TargetScoringPlaneHits"
    target_sp_passname: str = ""
    ecal_sp_coll_name: str = "EcalScoringPlaneHits"
    ecal_sp_passname: str = ""
    sim_particles_coll_name: str = "SimParticles"
    sim_particles_passname: str = ""
    sim_particles_event_passname: str = ""
    ecal_sp_hits_event_passname: str = ""
    target_sp_hits_event_passname: str = ""
