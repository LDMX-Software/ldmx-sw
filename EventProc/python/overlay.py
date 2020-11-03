"""Configuration for overlay """



from LDMX.Framework import ldmxcfg

class OverlayProducer(ldmxcfg.Producer) :
    """Configuration for pileup overlay

        Sets all parameters to reasonable defaults.

    Examples
    --------
        from LDMX.EventProc.overlay import OverlayProducer
        p.sequence.append( OverlayProducer() )
    """

    def __init__(self,name = 'overlay') :
        super().__init__(name,'ldmx::OverlayProducer','EventProc')


        self.overlayFileName = "ldmx_upstreamMultiElectron_events.root"
        self.passName = "sim"
        self.overlayPassName = "sim"
        self.overlayHitCollections=[ "TriggerPadUpSimHits", "EcalSimHits"]

        self.totalNumberOfInteractions = 2.
        self.timeSpread = 0.
        self.timeMean = 0.
        self.nBunchesToSample = 0.
        self.bunchSpacing = 26.88   # ns
        self.doPoisson = 0
        self.randomSeed=0
        self.verbosity=3

