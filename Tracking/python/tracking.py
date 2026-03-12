from math import sqrt

from LDMX.Framework.ldmxcfg import Producer
from LDMX.Tracking.make_path import makeFieldMapPath


class DigitizationProcessor(Producer):
    """ Producer that smears simulated tracker hits.

    Parameters
    ----------
    instance_name : str
        Unique name for this instance.


    Attributes
    ----------
    merge_hits : bool
        Activate merging of all hits that have the same track ID on the same
        layer.
    do_smearing : bool
        Activate the smearing.
    sigma_u : float
        Smearing sigma in the sensitive direction
    sigma_v : float
        Smearing sigma in the un-sensitive direction
    track_id : int
        If track_id > 0, retain only hits with that particular track_id and discard the rest.
    min_e_dep : float
        Minimum energy deposited by G4 to consider the hit
    hit_collection : string
        Input hit collection to be smeared
    out_collection : string
        Output hit collection to be stored
    """
    def __init__(self, instance_name="DigitizationProcessor"):
        super().__init__(instance_name,
                         'tracking::reco::DigitizationProcessor', 'Tracking')
        self.merge_hits = True
        self.do_smearing = True
        self.sigma_u = 0.006
        self.sigma_v = 0.000001
        self.track_id = -1
        self.min_e_dep = 0.05
        self.hit_collection = 'TaggerSimHits'
        self.out_collection = 'OutputMeasurements'
        self.tracker_hit_passname = ''

class SeedFinderProcessor(Producer):
    """ Producer to find Seeds for the KF-based track finding.

    Parameters
    ----------
    instance_name : str
        Unique name for this instance.

    Attributes
    ----------
    perigee_location : List[float]
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
    strategies : List[string] -- WORK IN PROGRESS AND NOT ACTIVE ---
        List of 5 hits (3 axial and 2 stereo) for seed finding.
    input_hits_collection : string
        The name of the input collection of hits to be used for seed finding.
    out_seed_collection : string
        The name of the ouput collection of seeds to be stored.
    u_error : float
        Uncertainty in the sensitive direction for the seed hits.
    v_error : float
        Uncertainty in the insensitive direction for the seed hits.
    """

    def __init__(self, instance_name="SeedFinderProcessor"):
        super().__init__(instance_name, 'tracking::reco::SeedFinderProcessor',
                         'Tracking')
        self.perigee_location = []
        self.pmin = 0.05
        self.pmax = 8.
        self.d0min = 20.
        self.d0max = 20.
        self.z0max = 60.
        self.phicut = 0.1
        self.thetacut = 0.2
        self.strategies = []
        self.bfield = 1.5
        self.input_hits_collection = 'TaggerSimHits'
        self.out_seed_collection = 'SeedTracks'
        self.input_pass_name = ''
        self.sim_particles_passname = ''
        self.tagger_trks_event_collection_passname = ''
        self.sim_particles_event_passname = ''
        self.u_error = 0.006
        self.v_error = 40. / sqrt(12)



class CKFProcessor(Producer):
    """ Producer that runs the Combinatorial Kalman Filter for track finding and fitting.

    Parameters
    ----------
    instance_name : str
        Unique name for this instance.

    Attributes
    ---------

    dumpobj : bool
        <functionality to be moved>
        If true, dump the tracking geometry into obj/mtl files for
        visualization purposes. The files can be opened via an open source
        application such as mesh lab.
    pionstates : int
        Can be used to define the number of pion states generated with uniform
        distributions to be propagated through the tracking geometry for
        debugging purposes. <functionality to be moved>
    bfield : float
        <functionality to be removed>
        If using a constant bfield, this is the BZ component.
    const_b_field : bool
        <functionality to be removed>
        Activate the usage of constant magnetic field.
    field_map_ : string
        Path to the location of the magnetic field map.
    propagator_step_size : float
        Size of each RK propagator step.
    propagator_maxSteps : int
        Maximum number of steps for the propagator
    perigee_location : list[double]
        DEPRECATED
    hit_collection : string
        The hit collection for pattern reconstruction.
    remove_stereo : bool
        Remove stereo hits from track fitting.
    use1Dmeasurements : bool
        <remove functionality and leave it to experts only>
        Use single strip measurements and not 3D points.
    min_hits : int
        Minimum number of measurements on track to accept the trajectory.
    use_extrapolate_location : bool
        Activate the usage of extrapolate location for returning the track
        parameters.
    extrapolate_location : list[double]
        Location of the extrapolation for the trajectory (perigee
        representation).
    use_seed_perigee : bool
        Uses the seed perigee as extrapolation location.
    seed_coll_name : string
        Seed collection for initiate the track finding.
    out_trk_collection : string
        Name of the output Track collection.
    do_smearing : bool
       <functionality to be removed>
       Activate the hit smearing.
    sigma_u : float
       <functionality to be removed>
       Smearing in the sensitive direction.
    sigma_v : float
       < functionality to be removed>
       Smearing in the unsensitive direction.
    kf_refit : bool
       Activate kalman filter track refitting of the found trajectories
    gsf_refit : bool
       <experimental>
       Refit tracks with Gaussian Sum Filter

    """

    def __init__(self, instance_name='CKFProcessor'):
        super().__init__(instance_name, 'tracking::reco::CKFProcessor',
                         'Tracking')

        self.dumpobj = False
        self.debug_acts = False
        self.pionstates = 0
        self.bfield = -1.5
        self.const_b_field = False
        self.field_map = makeFieldMapPath()
        self.propagator_step_size = 1000.
        self.propagator_maxSteps = 10000
        self.hit_collection = 'RecoilSimHits'
        self.remove_stereo = False
        self.use_extrapolate_location = True
        self.extrapolate_location = [0., 0., 0.]
        self.use_seed_perigee = False
        self.seed_coll_name = 'SeedTracks'
        self.out_trk_collection = 'Tracks'
        self.min_hits = 5
        self.outlier_pval_ = 3.84
        self.sim_particles_event_passname = ''
        self.input_pass_name = ''

class GSFProcessor(Producer):
    """ Producer that runs Gaussian Sum Fitter on a specific track collection

    Parameters
    ----------
    instance_name : str
        Unique name for this instance.

    Attributes
    ---------
    track_collection : string
        Track collection to be refitted with GSF
    meas_collection  : string
        Measurements collection in the tracker
    maxComponents   : int
        How many gaussians to use to sample the BetheHeitler
    abortOnError    : bool
        Abort fitting if an error occurred
    disableAllMaterialHandling : bool
        Disable material effects on surfaces. True only for debug purpose
    weightCutOff    : double
        Kill a component if its weight is smaller than a certain treshold.
    propagator_step_size : float
        Size of each RK propagator step.
    propagator_maxSteps : int
        Maximum number of steps for the propagator
    field_map_ : string
        Path to the location of the magnetic field map.
    """

    def __init__(self, instance_name='GSFProcessor'):
        super().__init__(instance_name, 'tracking::reco::GSFProcessor',
                         'Tracking')

        self.maxComponent    = 12
        self.abortOnError    = False
        self.disableAllMaterialHandling = False
        self.weightCutoff    = 1.0e-4
        self.debug = False

        self.propagator_step_size = 200.
        self.propagator_maxSteps  = 1000
        self.field_map = makeFieldMapPath()
        self.taggerTracking = True
        self.out_trk_collection = "GSFTracks"
        self.track_collection = "TaggerTracks"
        self.meas_collection = "DigiTaggerSimHits"


        self.track_passname = ""
        self.meas_passname = ""
        self.track_collection_event_passname = ""
        self.meas_collection_event_passname = ""




class TruthSeedProcessor(Producer):
    """ Producer that returns truth seeds to feed the KF based track finding.
    Seeds are not smeared, so the fits will be too optimistic, especially the
    residuals of the estimated locations w.r.t. simulated hits on each surface.
    The default parameters assume electron seeds are being found in the recoil
    tracker with loose requirements on momentum and z position.

    Parameters
    ----------
    instance_name : str
        Unique name for this instance.

    Attributes
    ----------

    pdg_ids : list[int]
        List of particle IDs whose scoring plane hits will be used to form
        initial seeds.
    scoring_hits_coll_name : string
        The name of the scoring plane hits from where to get the truth
        parameters.
    recoil_sim_sim_hits_coll_name : string
        The name of the sim tracker hits collection.
    n_min_hits_tagger : int
        The minimum number of hits to create a seed from in the tagger tracker.
    n_min_hits_recoil : int
        The minimum number of hits to create a seed from in the recoil tracker.
    z_min : double
        Request a minimum z (mm) for the scoring plane hits.
    track_id : int
        If positive, select only scoring hits with that particular track ID.
    pz_cut : double
        Minimum cut on the momentum (MeV)of the seed along the beam axis.
    p_cut : double
        Minimum cut on the momentum(MeV) of the seed.
    p_cut_max : double
        Maximum cut on the momentum of the seed.
    p_cut_ecal : double
        Minimum seed track momentum(MeV) at the ECAL scoring plane
    skip_tagger : bool
        Ignore the tagger tracker(makes empty collections).
    skip_recoil : bool
        Ignore the recoil tracker(makes empty collections).
    max_track_id : double
        Maximum track ID for a hit to be selected in the target scoring plane.
    """
    def __init__(self, instance_name = "TruthSeedProcessor"):
        super().__init__(instance_name, 'tracking::reco::TruthSeedProcessor',
                         'Tracking')

        self.pdg_ids = [11]
        self.scoring_hits_coll_name = 'TargetScoringPlaneHits'
        self.recoil_sim_hits_coll_name = 'RecoilSimHits'
        self.tagger_sim_hits_coll_name = 'TaggerSimHits'
        self.n_min_hits_tagger = 11
        self.n_min_hits_recoil = 7
        self.z_min = -9999. #mm
        self.track_id = -9999
        self.pz_cut = -9999. #MeV
        self.p_cut = 0. #MeV
        self.p_cut_max = 100000. #MeV
        self.p_cut_ecal = -1. #MeV
        self.skip_tagger = False
        self.skip_recoil = False
        self.max_track_id = 5

        self.sp_pass_name = ''
        self.input_pass_name = ''
        self.sim_particles_passname = ''
        self.particle_hypothesis = 11



class GreedyAmbiguitySolver(Producer):
    """ Producer that cleans duplicate tracks from CKF output.

    Parameters
    ----------
    instance_name : str
        Unique name for this instance.

    Attributes
    ----------

    Parameters
    ----------
    maximumSharedHits : int
        Maximum number of shared hits for a track to remain.
    maximumIterations : int
        Maximum number of iterations in track cleaning loop.
    nMeasurementsMin : int
        Minimum number of hits on a track.
    """
    def __init__(self, instance_name = "GreedyAmbiguitySolver"):
        super().__init__(instance_name, 'tracking::reco::GreedyAmbiguitySolver',
                         'Tracking')

        self.maximumSharedHits = 2
        self.maximumIterations = 1000
        self.nMeasurementsMin = 5
        self.out_trk_collection = "TaggerTracksClean"
        self.track_collection = "TaggerTracks"
        self.meas_collection = "DigiTaggerSimHits"

        self.input_pass_name = ""

class TrackerVetoProcessor(Producer):
    """ Class that flags events that pass the tracker veto
        This processor evaluates tracker events based on recoil and tagger track properties,
    applying configurable selection criteria to determine whether an event should be flagged.

    Parameters
    ----------
    instance_name : str
        Unique name for this instance.

    Attributes
    ----------
    max_d0 : float
        Maximum allowed d0 impact parameter for tracks.
    max_z0 : float
        Maximum allowed z0 impact parameter for tracks.
    max_chi2_per_ndf: float
        Max chi2/ndf required for tracks.
    min_recoil_n : int
        Minimum number of recoil tracks required.
    max_recoil_n : int
        Maximum number of recoil tracks allowed.
    min_tagger_momentum : float
        Minimum required momentum for tagger tracks.
    min_tagger_hits: int
          Min number of hits for tagger tracks required.
    min_recoil_hits: int
          Min number of hits for recoild tracks required.
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
    """

    def __init__(self, instance_name = "TrackerVetoProcessor"):
        super().__init__(instance_name, 'tracking::TrackerVetoProcessor','Tracking')


        self.max_d0 = 10.
        self.max_z0 = 40.0
        self.max_z0 = 40.0
        self.min_recoil_n = 1
        self.max_chi2_per_ndf = 5.0
        self.min_tagger_momentum = 5600.
        self.min_tagger_hits = 4
        self.min_recoil_hits = 4
        self.tagger_track_collection = "TaggerTracks"
        self.recoil_track_collection = "RecoilTracks"
        self.input_tagger_pass_name = ""
        self.input_recoil_pass_name = ""
        self.inverse_skim = False
        self.output_collection = "TrackerVeto"

        self.sim_particles_passname = ""
        self.input_collection_events_passname = ""


class StripFitProcessor(Producer):
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

    def __init__(self, instance_name="StripFitProcessor"):
        super().__init__(instance_name,
                         'tracking::reco::StripFitProcessor', 'Tracking')
        self.in_collection          = 'RawSiStripHits'
        self.in_pass                = ''
        self.out_collection         = 'FittedSiStripHits'
        self.t_scan_min_ns          = -50.0
        self.t_scan_max_ns          = 150.0
        self.t_scan_step_ns         = 1.0
        self.max_chi2_ndf           = -1.0


class StripClusterProcessor(Producer):
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

    def __init__(self, instance_name="StripClusterProcessor"):
        super().__init__(instance_name,
                         'tracking::reco::StripClusterProcessor', 'Tracking')
        self.in_collection          = 'FittedSiStripHits'
        self.in_pass                = ''
        self.out_collection         = 'StripMeasurements'
        self.seed_threshold         = 4.0
        self.neighbor_threshold     = 3.0
        self.cluster_threshold      = 4.0
        self.mean_time_ns           = 0.0
        self.time_window_ns         = -1.0
        self.neighbor_delta_t_ns    = -1.0
        self.max_chi2_ndf           = -1.0

