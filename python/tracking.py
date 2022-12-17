from LDMX.Framework.ldmxcfg import Producer


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
        super().__init__(instance_name, 'tracking::reco::DigitizationProcessor', 'Tracking')
        self.merge_hits = True
        self.do_smearing = True
        self.sigma_u = 0.06
        self.sigma_v = 0.0
        self.track_id = -1
        self.min_e_dep = 0.05
        self.hit_collection = 'TaggerSimHits'
        self.out_collection = 'OutputMeasurements'


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
    """

    def __init__(self, instance_name = "SeedFinderProcessor"):
        super().__init__(instance_name, 'tracking::sim::SeedFinderProcessor','Tracking')
        self.perigee_location = []
        self.pmin = 0.05
        self.pmax = 8.
        self.d0min = 20.
        self.d0max = 20.
        self.z0max = 60.
        self.strategies = []
        self.input_hits_collection = 'TaggerSimHits'
        self.out_seed_collection = 'SeedTracks'

class CKFProcessor(Producer):
    """ Producer that runs the Combinatorial Kalman Filter for track finding and fitting.

    Parameters
    ----------
    instance_name : str
        Unique name for this instance.

    Arguments
    ---------

    dumpobj : bool
        Activate dump of the tracking geometry into obj/mtl files for visualization purposes.
        The files can be opened via an open source application as mesh lab <functionality to be moved>
    pionstates : int
        Can be used to define the number of pion states generated with uniform distributions to be
        propagated through the tracking geometry for debugging purposes. <functionality to be moved>
    steps_output_file_path_ : string
        DEPRECATED TO BE REMOVED
    trackID : int
        Only keep the simulated hits with that particular track ID <functionality to be removed>
    pdgID : int
        Only keep the simulated hits with that particular pdg ID <functionality to be removed>
    bfield : float
        If using a constant bfield, this is the BZ component <functionality to be removed>
    const_b_field : bool
        Activate the usage of constant bField <functionality to be removed>
    bfieldMap_ : string
        Path to the location of the magnetic field map
    propagator_step_size : float
        Size of each RK propagator step
    propagator_maxSteps : int
        Maximum number of steps for the propagator
    perigee_location : list[double]
        DEPRECATED
    debug : bool
        Activate the debug printout
    hit_collection : string
        The hit collection for pattern reconstruction
    removeStereo : bool
        Remove stereo hits from track fitting
    use1Dmeasurements : bool
        Use single strip measurements and not 3D points. <remove functionality and leave it to experts only>
    minHits : int
        Minimum number of measurements on track to accept the trajectory.
    use_extrapolate_location : bool
        Activate the usage of extrapolate location for returning the track parameters
    extrapolate_location : list[double]
        Location of the extrapolation for the trajectory (perigee representation)
    use_seed_perigee : bool
        Uses the seed perigee as extrapolation location
    seed_coll_name : string
        Seed collection for initiate the track finding
    out_trk_collection : string
        Name of the output Track collection
    do_smearing : bool
       Activate the hit smearing <functionality to be removed>
    sigma_u : float
       Smearing in the sensitive direction <functionality to be removed>
    sigma_v : float
       Smearing in the unsensitive direction < functionality to be removed>

    kfRefit : bool
       Activate kalman filter track refitting of the found trajectories
    gsfRefit : bool
       Refit tracks with Gaussian Sum Filter <experimental>
        
    """
    def __init__(self, instance_name = 'CKFProcessor'): 
        super().__init__(instance_name, 'tracking::sim::CKFProcessor', 'Tracking')
        
class TruthSeedProcessor(Producer) :
    """ Producer that returns truth seeds to feed the KF based track finding.
    Seeds are not smeared, so the fits will be too optimistic, especially the residuals of the
    estimated locations w.r.t. simulated hits on each surface.

    Parameters
    ----------
    instance_name : str
        Unique name for this instance.

    Attributes
    ----------

    debug : bool
        Activate debug printouts
    out_trk_coll_name : string
        Name of the output seed collection
    pdgIDs : list[int]
        Only use the scoring plane hits with this particle ID to form initial seeds
    scoring_hits : string
        The name of the scoring plane hits from where to get the truth parameters
    sim_hit : string
        The name of the recoil sim hits collection 
    n_min_hits : int
        Request a seed to have at least
    z_min : double
        Request a minimum z for the scoring plan hits
    track_id : int
        If positive, select only scoring hits with that particular track id
    pz_cut : double
        Minimum cut on the momentum of the seed along the beam axis
    p_cut : double
        Minimum cut on the momentum of the seed
    p_cut_Max : double
        Maximum cut on the momentum of the seed
    k0_sel : bool
        Deprecated and to be removed
    p_cutEcal : double
        Minimum seed track momentum at the ECAL scoring plane
       
    
    """
    def __init__(self, instance_name = "TruthSeedProcessor"):
        super().__init__(instance_name, 'tracking::reco::TruthSeedProcessor','Tracking')

        self.debug = false;
        self.out_trk_coll_name = "TruthSeeds"
        self.pdgIDs = [11]
        self.scoring_hits = "TargetScoringPlaneHits_sim"
        self.sim_hits = "RecoilSimHits"
        self.n_min_hits = 7
        self.track_id = -999
        self.pz_cut = 0.
        self.p_cut = 0.
        self.p_cutMax = 8.
        self.k0_sel = False
        self.p_cutEcal = -1



