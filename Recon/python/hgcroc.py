"""Configuration for HGC ROC processors

Currently we only have an unpacker, but a packer
will also be written for validating raw data."""

from LDMX.Framework import ldmxcfg

class HgcrocUnpacker(ldmxcfg.Producer) :
    """Configuration for HgcrocUnpacker"""

    def __init__(self,input_name,output_name) :
        super().__init__(f'unpack_{input_name}','recon::HgcrocUnpacker','Recon')

        self.input_name = input_name
        self.input_pass = ''
        self.output_name = output_name
        self.roc_version = 2
