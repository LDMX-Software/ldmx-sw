from LDMX.Framework.ldmxcfg import Producer
from LDMX.Tracking.make_path import makeFieldMapPath
#from LDMX.Tracking.make_path import makeDetectorPath
                
class LinearSeedFinder(Producer):
    """ Producer to find Seeds for the reduced geometry track finding

    Parameters
    ----------
    instance_name : str
        Unique name for this instance.

    Attributes
    ----------
    input_hits_collection : string
        The name of the input collection of hits to be used for seed finding.
    input_recHits_collection : string
        The name of the input collection of Ecal RecHits (from layer 1) to give a degree of freedom to seed finding.
    out_seed_collection : string
        The name of the ouput collection of seeds to be stored.
    recoil_uncertainty : double
        The position uncertainty in [x, y] of a recoil tracker double-layer from combining an axial-stereo sensor pair to make one 3D position
    ecal_uncertainty : double
        The radius of an ECal hexagonal cell
    ecal_distance_threshold : double
        The maximum distance on the Ecal First Layer at which we still allow a seed to be saved
    layer12_midpoint : double
        The z coordinate of the midpoint between Recoil layers 1 and 2
    layer23_midpoint : double
        The z coordinate of the midpoint between Recoil layers 2 and 3
    layer34_midpoint : double
        The z coordinate of the midpoint between Recoil layers 3 and 4
    """

    def __init__(self, instance_name="LinearSeedFinder"):
        super().__init__(instance_name, 'tracking::reco::LinearSeedFinder', 'Tracking')
        self.input_hits_collection = 'DigiRecoilSimHits'
        self.input_rec_hits_collection = 'EcalRecHits'
        self.out_seed_collection = 'LinearRecoilSeedTracks'
        self.recoil_uncertainty = [0.006, 0.12]
        self.ecal_uncertainty = 3.87
        self.ecal_firstLayerZ_threshold = 250
        self.ecal_distance_threshold = 15.0
        self.layer12_midpoint = 12.5
        self.layer23_midpoint = 20.0
        self.layer34_midpoint = 27.5
        
class LinearTrackFinder(Producer):
    """ Producer to find Seeds for the reduced geometry track finding

    Parameters
    ----------
    instance_name : str
        Unique name for this instance.

    Attributes
    ----------
    seed_coll_name : string
        The name of the input collection of seeds to be used for track finding.
    out_trk_collection : string
        The name of the ouput collection of tracks to be stored.
    """

    def __init__(self, instance_name="LinearTrackFinder"):
        super().__init__(instance_name, 'tracking::reco::LinearTrackFinder', 'Tracking')
        self.seed_collection_ = 'LinearRecoilSeedTracks'
        self.out_trk_collection = 'LinearRecoilTracks'

class LinearTruthTracking(Producer):
    """ Producer to find (truth) tracks using trackID=1 for the reduced geometry

    Parameters
    ----------
    instance_name : str
        Unique name for this instance.

    Attributes
    ----------
    input_hits_collection : string
        The name of the input collection of hits to be used for truth tracking.
    input_recHits_collection : string
        The name of the input collection of Ecal RecHits (from layer 1) to check track quality
    out_track_collection : string
        The name of the ouput collection of truth tracks to be stored.
    recoil_uncertainty : double
        The position uncertainty in [x, y] of a recoil tracker double-layer from combining an axial-stereo sensor pair to make one 3D position
    layer12_midpoint : double
        The z coordinate of the midpoint between Recoil layers 1 and 2
    layer23_midpoint : double
        The z coordinate of the midpoint between Recoil layers 2 and 3
    layer34_midpoint : double
        The z coordinate of the midpoint between Recoil layers 3 and 4
    """

    def __init__(self, instance_name="LinearTruthTracking"):
        super().__init__(instance_name, 'tracking::reco::LinearTruthTracking', 'Tracking')
        self.input_hits_collection = 'DigiRecoilSimHits'
        self.input_rec_hits_collection = 'EcalRecHits'
        self.out_track_collection = 'LinearRecoilTruthTracks'
        self.recoil_truth_uncertainty = [0.006, 0.12]
        self.ecal_firstLayerZ_threshold = 250
        self.layer12_midpoint = 12.5
        self.layer23_midpoint = 20.0
        self.layer34_midpoint = 27.5
