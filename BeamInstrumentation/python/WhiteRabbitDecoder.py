from LDMX.Framework import ldmxcfg

class WhiteRabbitDecoder(ldmxcfg.Producer):
    def __init__(self, name = 'WhiteRabbitDecode'):
        super().__init__(f'{name}', 'beaminstrumentation::WhiteRabbitDecoder', "BeamInstrumentation")
        self.name = name
        self.input_pass_name = 'raw'
        self.input_collection = ''
        self.output_collection = ''
        self.is_real_data = True

    def decode():
        dec = WhiteRabbitDecoder()
        dec.input_collection = 'WRResults'
        dec.output_collection = 'decodedWRResults'

        return dec
    
