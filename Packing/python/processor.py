"""Configuration for the packing processor"""

from LDMX.Framework import ldmxcfg

class Unpacker(ldmxcfg.Producer) :
    """Processor to use to unpack from raw to digi.

    Parameters
    ----------
    translators : list[Translator]
        List of translators for decoding
    """

    def __init__(self, raw_file, translators = []) :
        super().__init__('unpacker','packing::Unpacker','Packing')
        self.translators = translators
        self.raw_file = raw_file
        self.raw_tree = 'LDMX_RawData'
        self.skip_unavailable = True
