"""TrigScint.QIEEncoder Python module

Sets all parameters to reasonable defaults.

Examples
--------
from LDMX.TrigScint.qieFormat import QIEDecoder
dec=QIEDecoder.tagger("$LDMX_BASE/ldmx-sw/TrigScint/util/channelMapFrontBack.txt")
p.sequence.extend([ dec ])

"""


from LDMX.Framework import ldmxcfg


class QIEEncoder(ldmxcfg.Producer) :
    """Configuration for QIE encoder"""
    def __init__(self, map_file, name = 'QIEEncode'):
        super().__init__(f'{name}', 'trigscint::QIEEncoder', "TrigScint")
        self.name = name
        self.input_pass_name = ''
        self.input_collection = 'trigScintQIEDigisTag'
        self.output_collection = 'QIEstreamTag'
        self.channel_map_file = map_file
        self.number_channels = 50
        self.verbose = False

    @staticmethod
    def tagger(map_file) :
        """Get the encoding emulator for the trigger pad upstream of tagger"""
        enc = QIEEncoder(map_file,'tag')
        enc.input_collection = 'trigScintQIEDigisTag'
        enc.output_collection= 'QIEstreamTag'
        return enc
    
    @staticmethod
    def up(map_file) :
        """Get the encoding emulator for the trigger pad upstream of target"""
        enc = QIEEncoder(map_file,'up')
        enc.input_collection = 'trigScintQIEDigisUp'
        enc.output_collection= 'QIEstreamUp'
        return enc

    @staticmethod
    def down(map_file) :
        """Get the encoding emulator for the trigger pad downstream of target"""
        enc = QIEEncoder(map_file,'down')
        enc.input_collection = 'trigScintQIEDigisDown'
        enc.output_collection= 'QIEstreamDown'
        return enc


class QIEDecoder(ldmxcfg.Producer) :
    """Configuration for QIE encoder"""
    def __init__(self, map_file, name = 'QIEDecode'):
        super().__init__(f'{name}', 'trigscint::QIEDecoder', "TrigScint")
        self.name = name
        self.input_pass_name = ''
        self.input_collection = 'QIEstreamTag'
        self.output_collection = 'decodedQIETag'
        self.channel_map_file = map_file
        self.number_channels = 50
        self.number_time_samples = 5
        self.is_real_data=False
        self.verbose = False

    @staticmethod
    def tagger(map_file) :
        """Get the decoding emulator for the trigger pad upstream of tagger"""
        dec = QIEDecoder(map_file,'tag')
        dec.input_collection = 'QIEstreamTag'
        dec.output_collection= 'decodedQIETag'
        return dec

    @staticmethod
    def up(map_file) :
        """Get the decoding emulator for the trigger pad upstream of target"""
        dec = QIEDecoder(map_file,'up')
        dec.input_collection = 'QIEstreamUp'
        dec.output_collection= 'decodedQIEUp'
        return dec

    @staticmethod
    def down(map_file) :
        """Get the decoding emulator for the trigger pad downstream of target"""
        dec = QIEDecoder(map_file,'down')
        dec.input_collection = 'QIEstreamDown'
        dec.output_collection= 'decodedQIEDown'
        return dec

