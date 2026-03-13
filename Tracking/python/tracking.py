from math import sqrt

from LDMX.Framework import Processor, field, processor

from .make_path import makeFieldMapPath


@processor("tracking::reco::DigitizationProcessor", "Tracking")
class DigitizationProcessor(Processor):
    """Producer that smears simulated tracker hits.

    Attributes
    ----------
    merge_hits : bool
        Activate merging of all hits that have the same track ID on the same
        layer.
    do_smearing : bool
        Activate the smearing.
    sigma_u : float
        Smearing sigma in the sensitive direction.
    sigma_v : float
        Smearing sigma in the un-sensitive direction.
    track_id : int
        If track_id > 0, retain only hits with that particular track_id and
        discard the rest.
    min_e_dep : float
        Minimum energy deposited by G4 to consider the hit.
    hit_collection : str
        Input hit collection to be smeared.
    out_collection : str
        Output hit collection to be stored.
    tracker_hit_passname : str
        The pass name of the tracker hits.
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
    propagator_maxSteps : int
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
    field_map: str = field(default_factory=makeFieldMapPath)
    propagator_step_size: float = 1000.0
    propagator_maxSteps: int = 10000
    hit_collection: str = "RecoilSimHits"
    remove_stereo: bool = False
    use_extrapolate_location: bool = True
    extrapolate_location: list[float] = [0.0, 0.0, 0.0]
    use_seed_perigee: bool = False
    seed_coll_name: str = "SeedTracks"
    out_trk_collection: str = "Tracks"
    min_hits: int = 5
    taggerTracking: bool = False
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
    maxComponent : int
        How many gaussians to use to sample the BetheHeitler.
    abortOnError : bool
        Abort fitting if an error occurred.
    disableAllMaterialHandling : bool
        Disable material effects on surfaces. True only for debug purpose.
    weightCutoff : float
        Kill a component if its weight is smaller than a certain threshold.
    debug : bool
        Enable debug output.
    propagator_step_size : float
        Size of each RK propagator step.
    propagator_maxSteps : int
        Maximum number of steps for the propagator.
    field_map : str
        Path to the location of the magnetic field map.
    taggerTracking : bool
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

    maxComponent: int = 12
    abortOnError: bool = False
    disableAllMaterialHandling: bool = False
    weightCutoff: float = 1.0e-4
    debug: bool = False
    propagator_step_size: float = 200.0
    propagator_maxSteps: int = 1000
    field_map: str = field(default_factory=makeFieldMapPath)
    taggerTracking: bool = True
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
    pdgIDs: list[int] = [11]
    scoring_hits: str = "TargetScoringPlaneHits"
    p_cutEcal: float = -1.0
    sp_pass_name: str = ""
    input_pass_name: str = ""
    sim_particles_coll_name: str = "SimParticles"
    sim_particles_passname: str = ""
    particle_hypothesis: int = 11
    beam_electrons_collection: str = "BeamElectrons"
    tagger_seeds_collection: str = "TaggerTruthSeeds"
    tagger_truth_collection: str = "TaggerTruth"
    recoil_seeds_collection: str = "RecoilTruthSeeds"
    recoil_truth_collection: str = "RecoilTruth"


@processor("tracking::reco::GreedyAmbiguitySolver", "Tracking")
class GreedyAmbiguitySolver(Processor):
    """Producer that cleans duplicate tracks from CKF output.

    Attributes
    ----------
    maximumSharedHits : int
        Maximum number of shared hits for a track to remain.
    maximumIterations : int
        Maximum number of iterations in track cleaning loop.
    nMeasurementsMin : int
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

    maximumSharedHits: int = 2
    maximumIterations: int = 1000
    nMeasurementsMin: int = 5
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
