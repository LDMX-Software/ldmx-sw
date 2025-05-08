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
        self.recoil_uncertainty = [0.006, 0.085]
        self.ecal_uncertainty = 3.87
        self.ecal_first_layer_z_threshold = 250.0
        self.ecal_distance_threshold = 15.0
        self.layer12_midpoint = 12.5
        self.layer23_midpoint = 20.0
        self.layer34_midpoint = 27.5
    
    def buildHistograms(self) :
        self.build1DHistogram("sensor1_measured_x-sim_stereo_layer1_x_ALL","Sensor1 Measured x - Sim Stereo1 x [mm]",200,-50,50)
        self.build1DHistogram("sensor1_measured_y-sim_stereo_layer1_y_ALL","Sensor1 Measured y - Sim Stereo1 y [mm]",200,-50,50)
        self.build1DHistogram("sensor2_measured_x-sim_stereo_layer2_x_ALL","Sensor2 Measured x - Sim Stereo2 x [mm]",200,-50,50)
        self.build1DHistogram("sensor2_measured_y-sim_stereo_layer2_y_ALL","Sensor2 Measured y - Sim Stereo2 y [mm]",200,-50,50)
        self.build1DHistogram("sensor1_measured_x-sim_stereo_layer1_x_same_trackID","Sensor1 Measured x - Sim Stereo1 x [mm]",100,-0.1,0.1)
        self.build1DHistogram("sensor1_measured_y-sim_stereo_layer1_y_same_trackID","Sensor1 Measured y - Sim Stereo1 y [mm]",100,-2,2)
        self.build1DHistogram("sensor2_measured_x-sim_stereo_layer2_x_same_trackID","Sensor2 Measured x - Sim Stereo2 x [mm]",100,-0.1,0.1)
        self.build1DHistogram("sensor2_measured_y-sim_stereo_layer2_y_same_trackID","Sensor2 Measured y - Sim Stereo2 y [mm]",100,-2,2)
        
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
    """

    def __init__(self, instance_name="LinearTruthTracking"):
        super().__init__(instance_name, 'tracking::reco::LinearTruthTracking', 'Tracking')
        self.input_hits_collection = 'RecoilSimHits'
        self.input_rec_hits_collection = 'EcalRecHits'
        self.out_track_collection = 'LinearRecoilTruthTracks'
        self.ecal_first_layer_z_threshold = 250.0
