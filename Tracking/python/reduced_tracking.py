from LDMX.Framework import Processor, processor


@processor("tracking::reco::LinearSeedFinder", "Tracking")
class LinearSeedFinder(Processor):
    """Producer to find Seeds for the reduced geometry track finding.

    Attributes
    ----------
    input_hits_collection : str
        The name of the input collection of hits to be used for seed finding.
    input_rec_hits_collection : str
        The name of the input collection of Ecal RecHits (from layer 1) to give
        a degree of freedom to seed finding.
    out_seed_collection : str
        The name of the output collection of seeds to be stored.
    recoil_uncertainty : list[float]
        The position uncertainty in [x, y] of a recoil tracker double-layer from
        combining an axial-stereo sensor pair to make one 3D position.
    ecal_uncertainty : float
        The radius of an ECal hexagonal cell.
    ecal_first_layer_z_threshold : float
        The z threshold for the first ECal layer.
    ecal_distance_threshold : float
        The maximum distance on the Ecal First Layer at which we still allow a
        seed to be saved.
    layer12_midpoint : float
        The z coordinate of the midpoint between Recoil layers 1 and 2.
    layer23_midpoint : float
        The z coordinate of the midpoint between Recoil layers 2 and 3.
    layer34_midpoint : float
        The z coordinate of the midpoint between Recoil layers 3 and 4.
    input_pass_name : str
        The pass name of the input collections.
    sim_particles_passname : str
        The pass name of the sim particles.
    sim_particles_events_passname : str
        The events pass name of the sim particles.
    """

    input_hits_collection: str = "DigiRecoilSimHits"
    input_rec_hits_collection: str = "EcalRecHits"
    out_seed_collection: str = "LinearRecoilSeedTracks"
    recoil_uncertainty: list[float] = [0.006, 0.085]
    ecal_uncertainty: float = 3.87
    ecal_first_layer_z_threshold: float = 250.0
    ecal_distance_threshold: float = 15.0
    layer12_midpoint: float = 12.5
    layer23_midpoint: float = 20.0
    layer34_midpoint: float = 27.5
    input_pass_name: str = ""
    sim_particles_passname: str = ""
    sim_particles_events_passname: str = ""


@processor("tracking::reco::LinearTrackFinder", "Tracking")
class LinearTrackFinder(Processor):
    """Producer to find tracks for the reduced geometry track finding.

    Attributes
    ----------
    seed_collection_ : str
        The name of the input collection of seeds to be used for track finding.
    out_trk_collection : str
        The name of the output collection of tracks to be stored.
    input_pass_name : str
        The pass name of the input collections.
    """

    seed_collection_: str = "LinearRecoilSeedTracks"
    out_trk_collection: str = "LinearRecoilTracks"
    input_pass_name: str = ""


@processor("tracking::reco::LinearTruthTracking", "Tracking")
class LinearTruthTracking(Processor):
    """Producer to find (truth) tracks using trackID=1 for the reduced geometry.

    Attributes
    ----------
    input_hits_collection : str
        The name of the input collection of hits to be used for truth tracking.
    input_rec_hits_collection : str
        The name of the input collection of Ecal RecHits (from layer 1) to check
        track quality.
    out_track_collection : str
        The name of the output collection of truth tracks to be stored.
    ecal_first_layer_z_threshold : float
        The z threshold for the first ECal layer.
    input_pass_name : str
        The pass name of the input collections.
    """

    input_hits_collection: str = "RecoilSimHits"
    input_rec_hits_collection: str = "EcalRecHits"
    out_track_collection: str = "LinearRecoilTruthTracks"
    ecal_first_layer_z_threshold: float = 250.0
    input_pass_name: str = ""
