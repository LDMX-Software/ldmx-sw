from LDMX.Framework.ldmxcfg import Producer


class DigitizationProcessor(Producer) :
    """ Producer that smears simulated tracker hits.

    Parameters
    ----------
    instance_name : str
        Unique name for this instance.


    Attributes
    ----------
    mergeHits : bool
        Activate merging of all hits that have the same trackID on the same layer.
    do_smearing : bool
        Activate the smearing.
    sigma_u : float
        Smearing sigma in the sensitive direction
    sigma_v : float
        Smearing sigma in the un-sensitive direction
    trackID : int
        If trackID>0, retain only hits with that particular trackID and discard the rest.
    minEdep : float
        Minimum energy deposited by G4 to consider the hit
    debug : bool
        Activate debug print-outs
    hit_collection : string
        Input hit collection to be smeared
    out_collection : string
        Output hit collection to be stored 
    """
    
    def __init__(self, instance_name = "DigitizationProcessor"):
        super().__init__(instance_name, 'tracking::reco::DigitizationProcessor','Tracking')
        self.mergeHits = True
        self.do_smearing = True
        self.sigma_u = 0.06
        self.sigma_v = 0.0
        self.trackID = -1
        self.debug = False
        self.minEdep = 0.05

class SeedFinderProcessor(Producer) :
    """ Producer to find Seeds for the KF-based track finding.

    Parameters
    ----------
    instance_name : str
        Unique name for this instance.

    Attributes
    ----------

    debug : bool
        Activate the debug print-out.
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
        self.debug = False
        self.pmax = 8.
        
        

class CKFProcessor(Producer) :
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
    """
    def __init__(self, instance_name = "TruthSeedProcessor"):
        super().__init__(instance_name, 'tracking::reco::TruthSeedProcessor','Tracking')

class VertexProcessor(Producer) :
    """ Producer to form vertices from a track collection. It could be used for K0 analysis and EN scattering.
    No vertex finding is in place, only vertex fitting. This producer is not fully validated and still work
    in progress. 

    Parameters
    ----------
    instance_name : str
        Unique name for this instance.
    """

    def __init__(self, instance_name = "VertexProcessor"):
        super().__init__(instance_name, 'tracking::reco::VertexProcessor','Tracking')

#This class is to produce vertices between two track collections, i.e. for tagger/recoil matching for example.
class Vertexer(Producer) :
    """ Producer that form vertices betwen two different track collections. It could be used for
    matching tagger/recoil tracks.

    Parameters
    ----------
    instance_name : str
        Unique name for this instance.
    """
    def __init__(self, instance_name = "Vertexer"):
        super().__init__(instance_name,'tracking::reco::Vertexer','Tracking')



