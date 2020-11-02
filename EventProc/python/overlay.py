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
        self.overlayTreeName = "LDMX_Events"
        self.totalNumberOfInteractions = 2.
        self.timeSpread = 0.
        self.doPoisson = 0
        self.overlayProcessName = "inclusive"
        self.overlayHitCollections=[ "TriggerPadUpSimHits", "EcalSimHits"]
        self.randomSeed=0
        self.verbosity=3

        self.pass_name = "sim"
        self.overlay_pass_name = "inclusive"
        self.collection_name = "inclusive"

#        import time
#        self.randomSeed = int(time.time())
        
