from math import sqrt

from LDMX.Framework import Processor, field, processor

from .make_path import make_field_map_path


@processor("tracking::reco::DigitizationProcessor", "Tracking")
class DigitizationProcessor(Processor):
    """Smears or fully digitizes simulated tracker hits.

    Mode 0 (use_charge_digitization=False): Gaussian smear in U/V directly to
    Measurement objects.

    Mode 1 (use_charge_digitization=True): Full silicon-strip charge simulation
    producing RawSiStripHit objects (and optionally Measurements), suitable for
    downstream StripFitProcessor + StripClusterProcessor.

    Attributes
    ----------
    merge_hits : bool
        Merge hits with the same track ID on the same layer.
    do_smearing : bool
        Activate Gaussian smearing (Mode 0 only).
    sigma_u : float
        Smearing sigma in the precision direction [mm] (Mode 0 only).
    sigma_v : float
        Smearing sigma in the strip direction [mm] (Mode 0 only).
    track_id : int
        If > 0, retain only hits with this track ID.
    min_e_dep : float
        Minimum energy deposit [MeV] to process a hit.
    hit_collection : str
        Input SimTrackerHit collection name.
    out_collection : str
        Output Measurement collection name.
    tracker_hit_passname : str
        Pass name of the input hit collection.
    use_charge_digitization : bool
        If True, run full charge simulation (Mode 1).
    bias_voltage : float
        Sensor bias voltage [V] (Mode 1).
    depletion_voltage : float
        Sensor depletion voltage [V] (Mode 1).
    noise_electrons : float
        Equivalent noise charge [e-] (Mode 1).
    threshold_electrons : float
        Strip hit threshold [e-] (Mode 1).
    is_n_type : bool
        True if sensor bulk is n-type (Mode 1).
    electron_side_readout : bool
        True if electrons are read out (Mode 1).
    hole_side_readout : bool
        True if holes are read out (Mode 1).
    electron_lorentz_tangent : float
        tan(theta_L) for electrons (Mode 1).
    hole_lorentz_tangent : float
        tan(theta_L) for holes (Mode 1).
    trapping : float
        Charge trapping fraction per 100 um drift (Mode 1).
    deposition_granularity : float
        Step size for charge deposition segments [fraction of sense pitch] (Mode 1).
    n_segments_min : int
        Minimum number of charge deposition segments (Mode 1).
    out_raw_collection : str
        Output RawSiStripHit collection name; empty disables (Mode 1).
    """

    merge_hits: bool = True
    do_smearing: bool = True
    sigma_u: float = 0.006
    sigma_v: float = 0.000001
    track_id: int = -1
    min_e_dep: float = 0.05
    hit_collection: str = "TaggerSimHits"
    out_collection: str = "OutputMeasurements"
    tracker_hit_passname: str = ""
    use_charge_digitization: bool = False
    use_lorentz: bool = True
    bias_voltage: float = 200.0
    depletion_voltage: float = 70.0
    noise_electrons: float = 1000.0
    threshold_electrons: float = 3000.0
    is_n_type: bool = False
    electron_side_readout: bool = False
    hole_side_readout: bool = True
    electron_lorentz_tangent: float = 0.0
    hole_lorentz_tangent: float = 0.0
    trapping: float = 0.0
    deposition_granularity: float = 0.10
    n_segments_min: int = 5
    out_raw_collection: str = ""


@processor("tracking::reco::SeedFinderProcessor", "Tracking")
class SeedFinderProcessor(Processor):
    """Producer to find Seeds for the KF-based track finding.

    Attributes
    ----------
    perigee_location : list[float]
        3D location of the perigee for the helix track parameters definition.
    pmin : float
        Minimum cut on the momentum of the seeds.
    pmax : float
        Maximum cut on the momentum of the seeds.
    d0min : float
        Minimum d0 allowed for the seeds. Computed at the perigee.
    d0max : float
        Maximum d0 allowed for the seeds. Computed at the perigee.
    z0max : float
        Maximum z0 allowed for the seeds. Computed at the perigee.
    phicut : float
        Cut on phi for seed finding.
    thetacut : float
        Cut on theta for seed finding.
    strategies : list[str]
        List of 5 hits (3 axial and 2 stereo) for seed finding.
    bfield : float
        Magnetic field strength.
    input_hits_collection : str
        The name of the input collection of hits to be used for seed finding.
    out_seed_collection : str
        The name of the output collection of seeds to be stored.
    input_pass_name : str
        The pass name of the input collections.
    sim_particles_coll_name : str
        The name of the sim particles collection.
    sim_particles_passname : str
        The pass name of the sim particles.
    tagger_trks_event_collection_passname : str
        The pass name of the tagger tracks event collection.
    sim_particles_event_passname : str
        The pass name of the sim particles event.
    u_error : float
        Uncertainty in the sensitive direction for the seed hits.
    v_error : float
        Uncertainty in the insensitive direction for the seed hits.
    """

    perigee_location: list[float] = []
    pmin: float = 0.05
    pmax: float = 8.0
    d0min: float = 20.0
    d0max: float = 20.0
    z0max: float = 60.0
    phicut: float = 0.1
    thetacut: float = 0.2
    strategies: list[str] = []
    bfield: float = 1.5
    input_hits_collection: str = "TaggerSimHits"
    out_seed_collection: str = "SeedTracks"
    input_pass_name: str = ""
    sim_particles_coll_name: str = "SimParticles"
    sim_particles_passname: str = ""
    tagger_trks_event_collection_passname: str = ""
    sim_particles_event_passname: str = ""
    u_error: float = 0.006
    v_error: float = 40.0 / sqrt(12)


@processor("tracking::reco::CKFProcessor", "Tracking")
class CKFProcessor(Processor):
    """Producer that runs the Combinatorial Kalman Filter for track finding
    and fitting.

    Attributes
    ----------
    dumpobj : bool
        If true, dump the tracking geometry into obj/mtl files for
        visualization purposes.
    debug_acts : bool
        Enable ACTS debug output.
    pionstates : int
        Number of pion states generated with uniform distributions to be
        propagated through the tracking geometry for debugging purposes.
    bfield : float
        If using a constant bfield, this is the BZ component.
    const_b_field : bool
        Activate the usage of constant magnetic field.
    field_map : str
        Path to the location of the magnetic field map.
    propagator_step_size : float
        Size of each RK propagator step.
    propagator_max_steps : int
        Maximum number of steps for the propagator.
    hit_collection : str
        The hit collection for pattern reconstruction.
    remove_stereo : bool
        Remove stereo hits from track fitting.
    use_extrapolate_location : bool
        Activate the usage of extrapolate location for returning the track
        parameters.
    extrapolate_location : list[float]
        Location of the extrapolation for the trajectory (perigee
        representation).
    use_seed_perigee : bool
        Uses the seed perigee as extrapolation location.
    seed_coll_name : str
        Seed collection for initiating the track finding.
    out_trk_collection : str
        Name of the output Track collection.
    min_hits : int
        Minimum number of measurements on track to accept the trajectory.
    outlier_pval_ : float
        Outlier p-value threshold.
    sim_particles_coll_name : str
        The name of the sim particles collection.
    sim_particles_event_passname : str
        The pass name of the sim particles event.
    input_pass_name : str
        The pass name of the input collections.
    """

    dumpobj: bool = False
    debug_acts: bool = False
    pionstates: int = 0
    bfield: float = -1.5
    const_b_field: bool = False
    field_map: str = ""
    propagator_step_size: float = 1000.0
    propagator_max_steps: int = 10000
    hit_collection: str = "RecoilSimHits"
    remove_stereo: bool = False
    use_extrapolate_location: bool = True
    extrapolate_location: list[float] = [0.0, 0.0, 0.0]
    use_seed_perigee: bool = False
    seed_coll_name: str = "SeedTracks"
    out_trk_collection: str = "Tracks"
    min_hits: int = 5
    tagger_tracking: bool = False
    measurement_collection: str = ""
    outlier_pval_: float = 3.84
    sim_particles_coll_name: str = "SimParticles"
    sim_particles_event_passname: str = ""
    input_pass_name: str = ""


@processor("tracking::reco::GSFProcessor", "Tracking")
class GSFProcessor(Processor):
    """Producer that runs Gaussian Sum Fitter on a specific track collection.

    Attributes
    ----------
    max_components : int
        How many gaussians to use to sample the BetheHeitler.
    abort_on_error : bool
        Abort fitting if an error occurred.
    disable_all_material_handling : bool
        Disable material effects on surfaces. True only for debug purpose.
    weight_cutoff : float
        Kill a component if its weight is smaller than a certain threshold.
    debug : bool
        Enable debug output.
    propagator_step_size : float
        Size of each RK propagator step.
    propagator_max_steps : int
        Maximum number of steps for the propagator.
    field_map : str
        Path to the location of the magnetic field map.
    tagger_tracking : bool
        Whether tracking in the tagger.
    out_trk_collection : str
        Name of the output Track collection.
    track_collection : str
        Track collection to be refitted with GSF.
    meas_collection : str
        Measurements collection in the tracker.
    track_passname : str
        The pass name of the track collection.
    meas_passname : str
        The pass name of the measurements collection.
    track_collection_event_passname : str
        The event pass name of the track collection.
    meas_collection_event_passname : str
        The event pass name of the measurements collection.
    """

    max_components: int = 12
    abort_on_error: bool = False
    disable_all_material_handling: bool = False
    weight_cutoff: float = 1.0e-4
    debug: bool = False
    propagator_step_size: float = 200.0
    propagator_max_steps: int = 1000
    field_map: str = ""
    tagger_tracking: bool = True
    out_trk_collection: str = "GSFTracks"
    track_collection: str = "TaggerTracks"
    meas_collection: str = "DigiTaggerSimHits"
    track_passname: str = ""
    meas_passname: str = ""
    track_collection_event_passname: str = ""
    meas_collection_event_passname: str = ""


@processor("tracking::reco::TruthSeedProcessor", "Tracking")
class TruthSeedProcessor(Processor):
    """Producer that returns truth seeds to feed the KF based track finding.

    Seeds are not smeared, so the fits will be too optimistic, especially the
    residuals of the estimated locations w.r.t. simulated hits on each surface.
    The default parameters assume electron seeds are being found in the recoil
    tracker with loose requirements on momentum and z position.

    Attributes
    ----------
    pdg_ids : list[int]
        List of particle IDs whose scoring plane hits will be used to form
        initial seeds.
    scoring_hits_coll_name : str
        The name of the scoring plane hits from where to get the truth
        parameters.
    recoil_sim_hits_coll_name : str
        The name of the sim tracker hits collection for recoil.
    tagger_sim_hits_coll_name : str
        The name of the sim tracker hits collection for tagger.
    n_min_hits_tagger : int
        The minimum number of hits to create a seed from in the tagger tracker.
    n_min_hits_recoil : int
        The minimum number of hits to create a seed from in the recoil tracker.
    z_min : float
        Request a minimum z (mm) for the scoring plane hits.
    track_id : int
        If positive, select only scoring hits with that particular track ID.
    pz_cut : float
        Minimum cut on the momentum (MeV) of the seed along the beam axis.
    p_cut : float
        Minimum cut on the momentum (MeV) of the seed.
    p_cut_max : float
        Maximum cut on the momentum of the seed.
    p_cut_ecal : float
        Minimum seed track momentum (MeV) at the ECAL scoring plane.
    skip_tagger : bool
        Ignore the tagger tracker (makes empty collections).
    skip_recoil : bool
        Ignore the recoil tracker (makes empty collections).
    max_track_id : float
        Maximum track ID for a hit to be selected in the target scoring plane.
    ecal_sp_coll_name : str
        The name of the ECAL scoring plane hits collection.
    sp_pass_name : str
        The pass name of the scoring plane hits.
    input_pass_name : str
        The pass name of the input collections.
    sim_particles_coll_name : str
        The name of the sim particles collection.
    sim_particles_passname : str
        The pass name of the sim particles.
    particle_hypothesis : int
        PDG ID for the particle hypothesis.
    beam_electrons_collection: str
        The name of the beam electrons collection to use
    tagger_seeds_collection : str
        The name of the tagger seeds collection to be stored.
    tagger_truth_collection : str
        The name of the tagger truth collection.
    recoil_seeds_collection : str
        The name of the recoil seeds collection.
    recoil_truth_collection : str
        The name of the recoil truth collection.
    """

    debug: bool = False
    pdg_ids: list[int] = [11]
    scoring_hits_coll_name: str = "TargetScoringPlaneHits"
    recoil_sim_hits_coll_name: str = "RecoilSimHits"
    tagger_sim_hits_coll_name: str = "TaggerSimHits"
    n_min_hits_tagger: int = 11
    n_min_hits_recoil: int = 7
    z_min: float = -9999.0
    track_id: int = -9999
    pz_cut: float = -9999.0
    p_cut: float = 0.0
    p_cut_max: float = 100000.0
    p_cut_ecal: float = -1.0
    skip_tagger: bool = False
    skip_recoil: bool = False
    max_track_id: int = 5
    ecal_sp_coll_name: str = "EcalScoringPlaneHits"
    trk_coll_name: str = ""
    pdg_ids: list[int] = [11]
    scoring_hits: str = "TargetScoringPlaneHits"
    p_cut_ecal: float = -1.0
    sp_pass_name: str = ""
    input_pass_name: str = ""
    sim_particles_coll_name: str = "SimParticles"
    sim_particles_passname: str = ""
    particle_hypothesis: int = 11
    beam_electrons_collection: str = "beamElectrons"
    tagger_seeds_collection: str = "TaggerTruthSeeds"
    tagger_truth_collection: str = "TaggerTruthTracks"
    recoil_seeds_collection: str = "RecoilTruthSeeds"
    recoil_truth_collection: str = "RecoilTruthTracks"


@processor("tracking::reco::GreedyAmbiguitySolver", "Tracking")
class GreedyAmbiguitySolver(Processor):
    """Producer that cleans duplicate tracks from CKF output.

    Attributes
    ----------
    maximum_shared_hits : int
        Maximum number of shared hits for a track to remain.
    maximum_iterations : int
        Maximum number of iterations in track cleaning loop.
    n_measurements_min : int
        Minimum number of hits on a track.
    out_trk_collection : str
        Name of the output Track collection.
    track_collection : str
        Track collection to be cleaned.
    meas_collection : str
        Measurements collection in the tracker.
    input_pass_name : str
        The pass name of the input collections.
    """

    maximum_shared_hits: int = 2
    maximum_iterations: int = 1000
    n_measurements_min: int = 5
    out_trk_collection: str = "TaggerTracksClean"
    track_collection: str = "TaggerTracks"
    meas_collection: str = "DigiTaggerSimHits"
    input_pass_name: str = ""


@processor("tracking::TrackerVetoProcessor", "Tracking")
class TrackerVetoProcessor(Processor):
    """Class that flags events that pass the tracker veto.

    This processor evaluates tracker events based on recoil and tagger track
    properties, applying configurable selection criteria to determine whether
    an event should be flagged.

    Attributes
    ----------
    max_d0 : float
        Maximum allowed d0 impact parameter for tracks.
    max_z0 : float
        Maximum allowed z0 impact parameter for tracks.
    max_chi2_per_ndf : float
        Max chi2/ndf required for tracks.
    min_recoil_n : int
        Minimum number of recoil tracks required.
    min_tagger_momentum : float
        Minimum required momentum for tagger tracks.
    min_tagger_hits : int
        Min number of hits for tagger tracks required.
    min_recoil_hits : int
        Min number of hits for recoil tracks required.
    tagger_track_collection : str
        The name of the tagger track collection.
    recoil_track_collection : str
        The name of the recoil track collection.
    input_tagger_pass_name : str
        The pass name of the input tagger collections.
    input_recoil_pass_name : str
        The pass name of the input recoil collections.
    inverse_skim : bool
        Boolean flag to invert the selection criteria for skimming purposes.
    output_collection : str
        The name of the new collection.
    sim_particles_passname : str
        The pass name of the sim particles.
    input_collection_events_passname : str
        The events pass name of the input collection.
    """

    max_d0: float = 10.0
    max_z0: float = 40.0
    max_chi2_per_ndf: float = 5.0
    min_recoil_n: int = 1
    min_tagger_momentum: float = 5600.0
    min_tagger_hits: int = 4
    min_recoil_hits: int = 4
    tagger_track_collection: str = "TaggerTracks"
    recoil_track_collection: str = "RecoilTracks"
    input_tagger_pass_name: str = ""
    input_recoil_pass_name: str = ""
    inverse_skim: bool = False
    output_collection: str = "TrackerVeto"
    sim_particles_passname: str = ""
    input_collection_events_passname: str = ""

@processor("tracking::reco::StripFitProcessor", "Tracking")
class StripFitProcessor(Processor):
    """Fits a pulse shape to each RawSiStripHit to extract amplitude and time.

    Applies to both real and simulated data.  The output FittedSiStripHit
    collection can be clustered downstream to produce Measurement objects
    for tracking.

    Attributes
    ----------
    in_collection : str
        Name of the input RawSiStripHit collection.
    in_pass : str
        Pass name for the input collection (empty = any).
    out_collection : str
        Name of the output FittedSiStripHit collection.
    t_scan_min_ns : float
        Lower bound of the hit-time search range [ns] (default -50).
    t_scan_max_ns : float
        Upper bound of the hit-time search range [ns] (default 150).
    t_scan_step_ns : float
        Step size of the coarse timing scan [ns] (default 1).
    max_chi2_ndf : float
        If > 0, discard fits with chi2/ndf above this value (default -1 = off).
    """

    in_collection: str = 'RawSiStripHits'
    in_pass: str = ''
    out_collection: str = 'FittedSiStripHits'
    t_scan_min_ns: float = -50.0
    t_scan_max_ns: float = 150.0
    t_scan_step_ns: float = 1.0
    max_chi2_ndf: float = -1.0


@processor("tracking::reco::TrackComparisonProcessor", "Tracking")
class TrackComparisonProcessor(Processor):
    """Compares tracking performance between a truth-smeared and a charge-digitized
    hit chain on a track-by-track basis.

    Tracks from two upstream collections are matched by their truth-matched
    SimParticle ID.  For each matched pair a row is written to a flat ROOT TTree
    and a set of quick-look TH1F histograms is filled.

    Attributes
    ----------
    trk_collection_smear : str
        Tagger truth-smeared track collection name.
    trk_collection_digi : str
        Tagger charge-digitized track collection name.
    pass_name_smear : str
        Pass name for the smeared tagger collection.
    pass_name_digi : str
        Pass name for the digi tagger collection.
    do_tagger : bool
        Enable tagger comparison.
    do_recoil : bool
        Enable recoil comparison.
    recoil_collection_smear : str
        Recoil truth-smeared track collection name.
    recoil_collection_digi : str
        Recoil charge-digitized track collection name.
    recoil_pass_smear : str
        Pass name for the smeared recoil collection.
    recoil_pass_digi : str
        Pass name for the digi recoil collection.
    min_truth_prob : float
        Minimum truth_prob required on both tracks to accept a pair.
    output_file : str
        Name of the output ROOT file containing the TTrees.
    """

    trk_collection_smear: str = "TaggerTracks"
    trk_collection_digi: str = "TaggerDigiTracks"
    pass_name_smear: str = ""
    pass_name_digi: str = ""
    do_tagger: bool = True
    do_recoil: bool = False
    recoil_collection_smear: str = "RecoilTracks"
    recoil_collection_digi: str = "RecoilDigiTracks"
    recoil_pass_smear: str = ""
    recoil_pass_digi: str = ""
    min_truth_prob: float = 0.5
    output_file: str = "track_comparison.root"


@processor("tracking::reco::StripClusterProcessor", "Tracking")
class StripClusterProcessor(Processor):
    """Clusters FittedSiStripHits into Measurements using nearest-neighbour clustering.

    Groups hits by sensor layer, runs nearest-neighbour BFS clustering on each
    layer (ported from HPS NearestNeighborRMSClusterer), and converts accepted
    clusters into ldmx::Measurement objects via the Acts tracking geometry.

    Applies to both real data (after StripFitProcessor) and simulation.

    Attributes
    ----------
    in_collection : str
        Input FittedSiStripHit collection (default "FittedSiStripHits").
    in_pass : str
        Pass name for the input collection (default "").
    out_collection : str
        Output Measurement collection (default "StripMeasurements").
    seed_threshold : float
        Minimum amplitude/noise_sigma to seed a cluster (default 4.0).
    neighbor_threshold : float
        Minimum amplitude/noise_sigma for a strip to join a cluster (default 3.0).
    cluster_threshold : float
        Minimum total_amp / sqrt(sum_noise^2) for cluster acceptance (default 4.0).
    mean_time_ns : float
        Expected hit time for the seed timing cut [ns] (default 0.0).
    time_window_ns : float
        Half-width of the seed timing window [ns]; <= 0 disables (default -1).
    neighbor_delta_t_ns : float
        Max |t0_neighbour - cluster_t| [ns] for a strip to join a cluster;
        <= 0 disables (default -1).
    max_chi2_ndf : float
        Max chi2/ndf for a fitted hit to be used; <= 0 disables (default -1).
    """

    in_collection: str = 'FittedSiStripHits'
    in_pass: str = ''
    out_collection: str = 'StripMeasurements'
    seed_threshold: float = 4.0
    neighbor_threshold: float = 3.0
    cluster_threshold: float = 4.0
    mean_time_ns: float = 0.0
    time_window_ns: float = -1.0
    neighbor_delta_t_ns: float = -1.0
    max_chi2_ndf: float = -1.0

