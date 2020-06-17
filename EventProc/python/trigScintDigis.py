"""Configuration for Trigger Scintillator digitization

Sets all parameters to reasonable defaults.

Examples
--------
>>> from LDMX.EventProc.trigScintDigis import *
>>> p.sequence.extend([ trigScintDigis , trigScintDigisDn , trigScintDigisTag ])
"""

from LDMX.Framework import ldmxcfg

class TrigScintDigiProducer(ldmxcfg.Producer) :
    """Configuration for digitizer for Trigger Scintillators"""

    def __init__(self,name) :
        super().__init__(name,'ldmx::TrigScintDigiProducer')

        from LDMX.EventProc import include
        include.library()

        self.mean_noise = 0.02
        self.number_of_strips = 50
        self.number_of_arrays = 1
        self.mev_per_mip = 0.4
        self.pe_per_mip = 10.
        self.input_collection="TriggerPadUpSimHits"
        self.output_collection="trigScintDigisUp"


trigScintDigisUp  = TrigScintDigiProducer("trigScintDigisUp")
trigScintDigisDn  = TrigScintDigiProducer("trigScintDigisDn")
trigScintDigisDn.input_collection = "TriggerPadDownSimHits"
trigScintDigisDn.output_collection = "trigScintDigisDn"

trigScintDigisTag = TrigScintDigiProducer("trigScintDigisTag")
trigScintDigisTag.input_collection = "TriggerPadTaggerSimHits"
trigScintDigisTag.output_collection = "trigScintDigisTag"

