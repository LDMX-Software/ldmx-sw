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
        self.verbose = False

    def tagger(mapFile) :
        """Get the decoding emulator for the trigger pad upstream of tagger"""
        enc = QIEDecoder(mapFile,'tag')
        enc.input_collection = 'QIEstreamTag'
        enc.output_collection= 'decodedQIETag'
        return enc
