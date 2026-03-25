"""Configuration for overlay

Parameters
----------------
overlay_filename : string
     The name of the file containing the pileup events to overlay on the sim event

Attributes:
-------------
sim_passname : string
    Pass name of the sim events
overlay_passname : string
    Pass name of the pileup events
calo_collections : string
    List of SimCalorimeterHit collections to pull from the sim
    and pileup events and combine
tracker_collections : string
    List of SimTrackerHit collections to pull from the sim
    and pileup events and combine
particle_collections : list[str]
    List of SimParticles collections to pull from the sim and pileup events and combine.
contrib_collections : list[str]
    List of SimCalorimeterHit collections which need contribs added separately.
poisson_mu : int
    The total number of interactions combined (including the sim event)
do_poisson_in_time : bool
    Specifies whether to sample a Poisson(totalNumberOfInteractions)
     to obtain the number of events to combine in-time
    with the sim event (doPoissonIntime=true),
    or deterministically set nOverlay=totalNumberOfInteractions-1
     (doPoissonIntime = false)
do_poisson_out_of_time : bool
    Specifies whether to sample a Poisson(totalNumberOfInteractions)
    to obtain the number of events to put in
    bunches that are out-of-time with the sim event (doPoissonOutoftime=true),
    or deterministically set nOverlay=totalNumberOfInteractions-1
     (doPoissonOutoftime = false)
time_sigma : float
    The width of a single bunch in time (expressed in sigma) [ns]
time_mean : float
    The average time, relative to the sim time, of the pileup events [ns].
    Note that this should generally be 0. A non-zero number combined
    with a 0 spread is useful for debugging.
n_earlier : int
    The number of preceding bunches sampled for out-of-time pileup.
    Furthermore, pileup will be uniformly distributed among this
    number of bunches m = -N_earlier, -N_earlier+1, ..., N_later
    while the sim event is always in bunch m = 0.
n_later : int
    The number of following bunches sampled for out-of-time pileup.
    Furthermore, pileup will be uniformly distributed among this
    number of bunches m = -N_earlier, -N_earlier+1, ..., N_later
    while the sim event is always in bunch m = 0.
bunch_spacing : float
    The spacing in time between bunches [ns]
start_event_min : int
    The minimum event number to start overlaying pileup events.
start_event_max : int
    The max event number to start overlaying pileup events.
track_id_encoding : int
    The version number for the track ID bitwise encoding schema.
    Possible values range over [0, 15].
    The version number is stored in bits 27-31 in the C++ int type.
"""

from LDMX.Framework import Processor, processor


@processor("recon::OverlayProducer", "Recon")
class OverlayProducer(Processor):
    """Configuration for pileup overlay

        Sets all parameters to reasonable defaults.

    Examples
    --------
        from LDMX.Recon.overlay import OverlayProducer
        p.sequence.append( OverlayProducer(
            overlay_filename='myPileupFileName.root' )
        )
    """

    overlay_filename: str = ""
    sim_passname: str = "sim"
    overlay_passname: str = "sim"
    calo_collections: list[str] = [
        "TriggerPad1SimHits",
        "TriggerPad2SimHits",
        "TriggerPad3SimHits",
        "TargetSimHits",
        "EcalSimHits",
        "HcalSimHits",
    ]
    tracker_collections: list[str] = [
        "TaggerSimHits",
        "RecoilSimHits",
        "EcalScoringPlaneHits",
        "TargetScoringPlaneHits"
    ]
    particle_collections: list[str] = ["SimParticles"]
    contrib_collections: list[str] = ["EcalSimHits", "HcalSimHits"]
    out_coll_postfix: str = "Overlay"
    poisson_mu: float = 2.0
    do_poisson_in_time: bool = False
    do_poisson_out_of_time: bool = False
    time_sigma: float = 0.0
    time_mean: float = 0.0
    n_earlier: int = 0
    n_later: int = 0
    bunch_spacing: float = 26.9
    tree_name: str = "LDMX_Events"
    start_event_min: int = 1
    start_event_max: int = 10000
    track_id_encoding: int = 1
