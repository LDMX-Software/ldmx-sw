"""Configuration for Trigger Scintillator digitization

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.EventProc.trigScintQIEDigis import TrigScintQIEDigiProducer
    p.sequence.extend([ TrigScintQIEDigiProducer.up() , TrigScintQIEDigiProducer.down() , TrigScintQIEDigiProducer.tagger() ])
"""

from LDMX.Framework import ldmxcfg

class TrigScintQIEDigiProducer(ldmxcfg.Producer) :
    """Configuration for digitizer for Trigger Scintillators"""

    def __init__(self,name) :
        super().__init__(name,'ldmx::TrigScintQIEDigiProducer','EventProc')

        self.mean_noise = 0.02
        self.number_of_strips = 50
        self.number_of_arrays = 1
        self.mev_per_mip = 0.4
        self.pe_per_mip = 100.
        self.input_collection="TriggerPadUpSimHits"
        self.input_pass_name="" #take any pass
        self.output_collection="trigScintQIEDigisUp"
        import time
        self.randomSeed = int(time.time())
        self.verbose = False

    def up() :
        """Get the digitizer for the trigger pad upstream of target"""
        digi = TrigScintQIEDigiProducer( 'trigScintQIEDigisUp' )
        digi.input_collection = 'TriggerPadUpSimHits'
        digi.output_collection= 'trigScintQIEDigisUp'
        return digi

    def down() :
        """Get the digitizer for the trigger pad downstream of target"""
        digi = TrigScintQIEDigiProducer( 'trigScintQIEDigisDn' )
        digi.input_collection = 'TriggerPadDownSimHits'
        digi.output_collection= 'trigScintQIEDigisDn'
        return digi

    def tagger() :
        """Get the digitizer for the trigger pad upstream of tagger"""
        digi = TrigScintQIEDigiProducer( 'trigScintQIEDigisTag' )
        digi.input_collection = 'TriggerPadTaggerSimHits'
        digi.output_collection= 'trigScintQIEDigisTag'
        return digi

