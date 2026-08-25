from LDMX.Framework.ldmxcfg import Producer


class RawTrackerDecoder(Producer):
    """Decode raw Rogue frame bytes into a RawSiStripHit collection.

    Attributes
    ----------
    input_collection : str
        Raw byte buffer collection name from SingleSubsystemUnpacker.
    input_pass_name : str
        Pass name of the upstream producer (empty = any pass).
    output_collection : str
        Name for the output RawSiStripHit collection on the event bus.
    n_samples : int
        ADC samples per hit (always 3 for APV25).
    """

    def __init__(self, name="raw_tracker_decoder"):
        super().__init__(name, "tracking::reco::RawTrackerDecoder", "Tracking")
        self.input_collection = "TrackerRawData"
        self.input_pass_name = ""
        self.output_collection = "RawSiStripHits"
        self.n_samples = 3
