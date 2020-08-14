"""Configuration for Trigger Scintillator track producer 

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.EventProc.trigScintTracking import trigScintTrack
    p.sequence.extend([ TrigScintTrackProducer ] )
"""

from LDMX.Framework import ldmxcfg

class TrigScintTrackProducer(ldmxcfg.Producer) :
    """Configuration for track producer for Trigger Scintillators"""

    def __init__(self,name) :
        super().__init__(name,'ldmx::TrigScintTrackProducer')

        from LDMX.EventProc import include
        include.library()

        self.delta_max = 1.0
        self.tracking_threshold = 0.  #to add in neighboring channels
        self.seeding_collection = "TriggerPadTaggerClusters"
        self.further_input_collections = ["TriggerPadUpClusters","TriggerPadDownClusters"]
        self.input_pass_name="" #take any pass
        self.output_collection="TriggerPadTracks"
        self.verbosity = 0



trigScintTrack = TrigScintTrackProducer( "trigScintTrack" )
