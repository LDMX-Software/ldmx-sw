from LDMX.Framework import ldmxcfg

class FiberTrackerDecoder(ldmxcfg.Producer):
    def __init__(self, name = 'FiberTrackerDecode'):
        super().__init__(f'{name}', 'beaminstrumentation::FiberTrackerDecoder', "BeamInstrumentation")
        self.name = name
        self.input_pass_name = 'raw'
        self.input_collection = ''
        self.output_collection = ''
        self.is_real_data = True

    def decode():
        dec = FiberTrackerDecoder()
        dec.input_collection_downstream_horizontal = 'FiberTrackerFT50'
        dec.input_collection_downstream_vertical = 'FiberTrackerFT51'
        dec.input_collection_upstream_horizontal = 'FiberTrackerFT41'
        dec.input_collection_upstream_vertical = 'FiberTrackerFT42'
        dec.output_collection = 'decodedFiberTracker'

        return dec
    
