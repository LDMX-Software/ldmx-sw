"""TrigScint.ZCCMEncoder Python module

Sets all parameters to reasonable defaults.

Examples
--------
from LDMX.TrigScint.zccmFormat import ZCCMDecoder
dec=ZCCMDecoder.tagger("$LDMX_BASE/ldmx-sw/TrigScint/util/channelMapFrontBack.txt")
p.sequence.extend([ dec ])

"""

#NOTE this needs rewriting substantially to deal with the fact that we're doing all modules in one stream

from LDMX.Framework import ldmxcfg


class ZCCMEncoder(ldmxcfg.Producer) :
    """Configuration for ZCCM encoder"""
    def __init__(self, channelMapFile, name = 'ZCCMEncode'):
        super().__init__(f'{name}', 'trigscint::ZCCMEncoder', "TrigScint")
        self.name = name
        self.input_pass_name = ''
        self.input_collection = 'trigScintZCCMDigis'
        self.output_collection = 'ZCCMDigis'
        self.module_map_file = moduleMapFile
        self.channel_map_file = channelMapFile
        self.number_channels = 14*6

    def tagger(mapFile) :
        """Get the encoding emulator for the trigger pad upstream of tagger"""
        enc = ZCCMEncoder(mapFile,'tag')
        enc.input_collection = 'trigScintZCCMDigisTag'
        enc.output_collection= 'ZCCMstreamTag'
        return enc

    def up(mapFile) :
        """Get the encoding emulator for the trigger pad upstream of target"""
        enc = ZCCMEncoder(mapFile,'up')
        enc.input_collection = 'trigScintZCCMDigisUp'
        enc.output_collection= 'ZCCMstreamUp'
        return enc

    def down(mapFile) :
        """Get the encoding emulator for the trigger pad downstream of target"""
        enc = ZCCMEncoder(mapFile,'down')
        enc.input_collection = 'trigScintZCCMDigisDown'
        enc.output_collection= 'ZCCMstreamDown'
        return enc


class ZCCMDecoder(ldmxcfg.Producer) :
    """Configuration for ZCCM decoder"""
    def __init__(self, mapFile, name = 'ZCCMDecode'):
        super().__init__(f'{name}', 'trigscint::ZCCMDecoder', "TrigScint")
        self.name = name
        self.input_pass_name = ''
        self.input_collection = 'ZCCMoutput'
        self.output_collection = 'decodedZCCM'
        self.channel_map_file = mapFile
        self.number_channels = 14*6
        self.number_time_samples = 70
        self.is_real_data=True

    def tagger(mapFile) :
        """Get the decoding emulator for the trigger pad upstream of tagger"""
        dec = ZCCMDecoder(mapFile,'tag')
        dec.input_collection = 'ZCCMstreamTag'
        dec.output_collection= 'decodedZCCMTag'
        return dec

    def up(mapFile) :
        """Get the decoding emulator for the trigger pad upstream of target"""
        dec = ZCCMDecoder(mapFile,'up')
        dec.input_collection = 'ZCCMoutputUp'
        dec.output_collection= 'decodedZCCMUp'
        return dec

    def down(mapFile) :
        """Get the decoding emulator for the trigger pad downstream of target"""
        dec = ZCCMDecoder(mapFile,'down')
        dec.input_collection = 'ZCCMstreamDown'
        dec.output_collection= 'decodedZCCMDown'
        return dec

