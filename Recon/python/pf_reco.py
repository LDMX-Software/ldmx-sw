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
    max_shared_frac: float = 0.5  # drop tracks sharing more hits with a better one
    output_track_coll_name: str = "PFTracks"


@processor("recon::ParticleFlow", "Recon")
class PFProducer(Processor):
    """Configuration for particle reco

    By default the Ecal input is CLUE3D without superclustering, from
    pf_ecal_clusters(), and satellites are merged by Ecal to Ecal linking.
    """

    input_ecal_coll_name: str = "PFEcalClustersCLUE3D"
    input_hcal_coll_name: str = "PFHcalClusters"
    input_track_coll_name: str = "PFTracks"
    use_existing_ecal_clusters: bool = True
    output_coll_name: str = "PFCandidates"
    single_particle: bool = False
    input_ecal_passname: str = ""
    input_hcal_passname: str = ""
    input_tracks_passname: str = ""
    truth_sp_coll_name: str = "EcalScoringPlaneHits"
    truth_sp_passname: str = ""
    # dE/dx estimates made on the PF tracks, empty name disables
    input_mass_coll_name: str = "PFTrackDeDx"
    input_mass_passname: str = ""
    # linking and particle ID
    link_max_dist: float = 2.0  # RMS-normalized
    link_min_rms: float = 5.0  # mm, floor on the cluster RMS
    electron_ep_min: float = 0.7
    excess_ep: float = 1.6
    # dE/dx m2 [MeV^2] above proton_min is p, kaon_min to proton_min is K;
    # only below the momentum limits [MeV], m2 resolution grows as p^2
    proton_min_mass2: float = 4.0e5
    kaon_min_mass2: float = 1.2e5
    proton_max_p: float = 1500.0
    kaon_max_p: float = 1000.0
    # low E/p tracks absorb unlinked Ecal clusters within the radius until
    # E/p reaches recovery_stop_ep; recovers split electron showers, 0 disables
    recovery_radius: float = 30.0  # mm
    recovery_stop_ep: float = 0.85
    # Ecal to Ecal linking of satellites, for CLUE3D clusters without
    # superclustering; radius 0 disables. Same rules as clue3d_sc_*
    ecal_link_radius: float = 150.0  # mm
    ecal_link_ratio: float = 0.06
    ecal_link_min_layers: int = 5
    # soft clusters that stay photons: E [MeV], isolation and start z [mm]
    photon_keep_min_e: float = 100.0
    photon_keep_min_dist: float = 35.0
    photon_keep_max_zstart: float = 330.0
    dedx_min_hits: int = 3
    # energy corrections vs raw cluster energy [MeV], empty means none
    # old Ecal table overcorrected electrons, disabled until rederived
    ecal_corr_energies: list[float] = []
    ecal_corr_factors: list[float] = []
    # Hcal raw energy is scintillator only; factors from neutron guns
    # (0.5 to 4 GeV): median KE / raw merged energy, scaled 0.9 to remove
    # the overshoot seen after merging
    hcal_corr_energies: list[float] = [7.7, 27.3, 61.7, 168.3]
    hcal_corr_factors: list[float] = [58.7, 32.9, 29.2, 21.4]
    # Hcal to Hcal linking cone from the target, 0 disables
    hcal_merge_angle: float = 40.0  # deg
    # track to Hcal link cone from the track at the Ecal face, 0 disables
    hcal_track_angle: float = 25.0  # deg
    # energy subtraction: charged hadrons absorb free Ecal clusters within the
    # radius and Hcal clusters in their cone while calo E < absorb_ep * p
    hadron_ecal_radius: float = 100.0  # mm
    hadron_absorb_ep: float = 1.5
    # drop neutral candidates below this energy, mostly soft fragments
    min_neutral_e: float = 30.0  # MeV
    # electron shower depth limit: max_depth at 1 GeV, + slope per doubling of p
    ecal_front_z: float = 248.0  # mm, first Ecal layer
    electron_max_depth: float = 130.0  # mm
    electron_depth_slope: float = 12.0  # mm


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


def pf_ecal_clusters(**kwargs):
    """CLUE3D Ecal clusters for PF, superclustering left to PF linking"""
    from LDMX.Ecal.ecal_clusters import EcalClusterProducer

    return EcalClusterProducer(
        instance_name="PFEcalClusterCLUE3D",
        cluster_coll_name="PFEcalClustersCLUE3D",
        algo_coll_name="PFClusterAlgoResultCLUE3D",
        algorithm="clue3d",
        clue3d_sc_radius=0.0,
        **kwargs,
    )


def pf_track_dedx(**kwargs):
    """dE/dx estimates on the PF tracks, indices match for PF"""
    from LDMX.Recon.track_dedx_mass_estimator import TrackDeDxMassEstimator

    return TrackDeDxMassEstimator(
        instance_name="PFTrackDeDxEstimator",
        track_collection="PFTracks",
        out_collection="PFTrackDeDx",
        **kwargs,
    )
