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
    def __init__(self, mapFile, name = 'QIEEncode'): 
        super().__init__(f'{name}', 'trigscint::QIEEncoder', "TrigScint")
        self.name = name 
        self.input_pass_name = ''
        self.input_collection = 'trigScintQIEDigisTag'
        self.output_collection = 'QIEstreamTag'
        self.channel_map_file = mapFile 
        self.number_channels = 50
        self.verbose = False

    def tagger(mapFile) :
        """Get the encoding emulator for the trigger pad upstream of tagger"""
        enc = QIEEncoder(mapFile,'tag')
        enc.input_collection = 'trigScintQIEDigisTag'
        enc.output_collection= 'QIEstreamTag'
        return enc

    def up(mapFile) :
        """Get the encoding emulator for the trigger pad upstream of target"""
        enc = QIEEncoder(mapFile,'up')
        enc.input_collection = 'trigScintQIEDigisUp'
        enc.output_collection= 'QIEstreamUp'
        return enc

    def down(mapFile) :
        """Get the encoding emulator for the trigger pad downstream of target"""
        enc = QIEEncoder(mapFile,'down')
        enc.input_collection = 'trigScintQIEDigisDown'
        enc.output_collection= 'QIEstreamDown'
        return enc


class QIEDecoder(ldmxcfg.Producer) :
    """Configuration for QIE encoder"""
    def __init__(self, mapFile, name = 'QIEDecode'): 
        super().__init__(f'{name}', 'trigscint::QIEDecoder', "TrigScint")
        self.name = name 
        self.input_pass_name = ''
        self.input_collection = 'QIEstreamTag'
        self.output_collection = 'decodedQIETag'
        self.channel_map_file = mapFile
        self.number_channels = 50
        self.number_time_samples = 5
        self.verbose = False

    def tagger(mapFile) :
        """Get the decoding emulator for the trigger pad upstream of tagger"""
        dec = QIEDecoder(mapFile,'tag')
        dec.input_collection = 'QIEstreamTag'
        dec.output_collection= 'decodedQIETag'
        return dec

    def up(mapFile) :
        """Get the decoding emulator for the trigger pad upstream of target"""
        dec = QIEDecoder(mapFile,'up')
        dec.input_collection = 'QIEstreamUp'
        dec.output_collection= 'decodedQIEUp'
        return dec

    def down(mapFile) :
        """Get the decoding emulator for the trigger pad downstream of target"""
        dec = QIEDecoder(mapFile,'down')
        dec.input_collection = 'QIEstreamDown'
        dec.output_collection= 'decodedQIEDown'
        return dec

