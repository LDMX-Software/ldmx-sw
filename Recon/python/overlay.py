"""Configuration for overlay 

Parameters
----------------
fileName : string
     The name of the file containing the pileup events to overlay on the sim event

Attributes:
-------------
passName : string
    Pass name of the sim events 
overlayPassName : string
    Pass name of the pileup events 
overlayCaloHitCollections : string
    List of SimCalorimeterHit collections to pull from the sim and pileup events and combine
overlayTrackerHitCollections : string
    List of SimTrackerHit collections to pull from the sim and pileup events and combine
totalNumberOfInteractions : int 
    The total number of interactions combined (including the sim event)
doPoisson : int
    Specifies whether to sample a Poisson(totalNumberOfInteractions) to obtain the number of events to combine (doPoisson != 0), 
    or deterministically set nOverlay=totalNumberOfInteractions-1 (doPoisson = 0)
timeSpread : float
    The width of a single bunch in time (expressed in sigma) [ns]
timeMean : float
    The average time, relative to the sim time, of the pileup events [ns]. 
    Note that this should generally be 0. A non-zero number combined with a 0 spread is useful for debugging.
nEarlierBunchesToSample : int
    The number of preceding bunches sampled for out-of-time pileup. 
    Furthermore, pileup will be uniformly distributed among this number of bunches m = -N_earlier, -N_earlier+1, ..., N_later 
    while the sim event is always in bunch m = 0. 
nLatererBunchesToSample : int
    The number of following bunches sampled for out-of-time pileup. 
    Furthermore, pileup will be uniformly distributed among this number of bunches m = -N_earlier, -N_earlier+1, ..., N_later 
    while the sim event is always in bunch m = 0. 
bunchSpacing : float
    The spacing in time between bunches [ns]
verbosity : int
    Sets the producer specific level of verbosity, up to 3 for the most verbose step-by-step debug printouts.

    
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

    def __init__(self,fileName,name = 'OverlayProducer') :
        super().__init__(name,'recon::OverlayProducer','Recon')


        self.overlayFileName = fileName 
        self.passName = "sim"
        self.overlayPassName = "sim"
        self.overlayCaloHitCollections=[ "TriggerPadUpSimHits", "EcalSimHits"]
        self.overlayTrackerHitCollections=[ "TaggerSimHits"]
        self.totalNumberOfInteractions = 2.
        self.doPoisson = False
        self.timeSpread = 0.        # [ns]
        self.timeMean = 0.          # [ns]
        self.nEarlierBunchesToSample = 0
        self.nLaterBunchesToSample = 0
        self.bunchSpacing = 26.88   # [ns]
        self.verbosity = 1

