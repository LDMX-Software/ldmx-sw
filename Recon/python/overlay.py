"""Configuration for overlay

Parameters
----------------
file_name : string
     The name of the file containing the pileup events to overlay on the sim event

Attributes:
-------------
sim_passname : string
    Pass name of the sim events
overlay_passname : string
    Pass name of the pileup events
calo_collections : string
    List of SimCalorimeterHit collections to pull from the sim and pileup events and combine
tracker_collections_ : string
    List of SimTrackerHit collections to pull from the sim and pileup events and combine
poisson_mu : int
    The total number of interactions combined (including the sim event)
do_poisson_in_time : bool
    Specifies whether to sample a Poisson(totalNumberOfInteractions) to obtain the number of events to combine in-time
    with the sim event (doPoissonIntime=true),
    or deterministically set nOverlay=totalNumberOfInteractions-1 (doPoissonIntime = false)
do_poisson_out_of_time : bool
    Specifies whether to sample a Poisson(totalNumberOfInteractions) to obtain the number of events to put in
    bunches that are out-of-time with the sim event (doPoissonOutoftime=true),
    or deterministically set nOverlay=totalNumberOfInteractions-1 (doPoissonOutoftime = false)
time_sigma : float
    The width of a single bunch in time (expressed in sigma) [ns]
time_mean : float
    The average time, relative to the sim time, of the pileup events [ns].
    Note that this should generally be 0. A non-zero number combined with a 0 spread is useful for debugging.
n_earlier : int
    The number of preceding bunches sampled for out-of-time pileup.
    Furthermore, pileup will be uniformly distributed among this number of bunches m = -N_earlier, -N_earlier+1, ..., N_later
    while the sim event is always in bunch m = 0.
n_later : int
    The number of following bunches sampled for out-of-time pileup.
    Furthermore, pileup will be uniformly distributed among this number of bunches m = -N_earlier, -N_earlier+1, ..., N_later
    while the sim event is always in bunch m = 0.
bunch_spacing : float
    The spacing in time between bunches [ns]
start_event_min : int
    The minimum event number to start overlaying pileup events.
start_event_max : int
    The max event number to start overlaying pileup events.
track_id_encoding : int
    The version number for the track ID bitwise encoding schema. Possible values range over [0, 15]. The version number is stored in bits 27-31 in the C++ int type.
"""

from LDMX.Framework import ldmxcfg


class OverlayProducer(ldmxcfg.Producer) :
    """Configuration for pileup overlay

        Sets all parameters to reasonable defaults.

    Examples
    --------
        from LDMX.Recon.overlay import OverlayProducer
        p.sequence.append( OverlayProducer( myPileupFileName.root ) )
    """

    def __init__(self,file_name,name = 'OverlayProducer') :
        super().__init__(name,'recon::OverlayProducer','Recon')


        self.overlay_filename = file_name
        self.sim_passname = "sim"
        self.overlay_passname = "sim"
        self.calo_collections =  ["TriggerPad1SimHits", "TriggerPad2SimHits", "TriggerPad3SimHits",
                                   "TargetSimHits", "EcalSimHits", "HcalSimHits"]
        self.tracker_collections = [ "TaggerSimHits", "RecoilSimHits", "EcalScoringPlaneHits",
                                   "TargetScoringPlaneHits" ]
        self.particle_collections = [ "SimParticles" ]
        self.contrib_collections = [ "EcalSimHits", "HcalSimHits" ]
        self.out_coll_postfix = "Overlay"
        self.poisson_mu = 2.
        self.do_poisson_in_time = False
        self.do_poisson_out_of_time = False
        # realistically, 30 ps;
        self.time_sigma = 0.        # [ns]
        self.time_mean = 0.          # [ns]
        self.n_earlier = 0
        self.n_later = 0
        # 26.9 = 1000./37.2 ; 5.4 = 1000./186.
        self.bunch_spacing = 26.9   # [ns]
        self.tree_name = 'LDMX_Events'
        self.start_event_min = 1
        self.start_event_max = 10000
        self.track_id_encoding = 1
