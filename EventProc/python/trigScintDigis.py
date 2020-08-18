"""Configuration for Trigger Scintillator digitization

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.EventProc.trigScintDigis import TrigScintDigiProducer
    p.sequence.extend([ TrigScintDigiProducer.up() , TrigScintDigiProducer.down() , TrigScintDigiProducer.tagger() ])
"""

from LDMX.Framework import ldmxcfg

class TrigScintDigiProducer(ldmxcfg.Producer) :
    """Configuration for digitizer for Trigger Scintillators"""

    def __init__(self,name) :
        super().__init__(name,'ldmx::TrigScintDigiProducer','EventProc')

        self.mean_noise = 0.02
        self.number_of_strips = 50
        self.number_of_arrays = 1
        self.mev_per_mip = 0.4
        self.pe_per_mip = 100.
        self.input_collection="TriggerPadUpSimHits"
        self.input_pass_name="" #take any pass
        self.output_collection="trigScintDigisUp"
        import time
        self.randomSeed = int(time.time())
        self.verbose = False

    def up() :
        """Get the digitizer for the trigger pad upstream of target"""
        digi = TrigScintDigiProducer( 'trigScintDigisUp' )
        digi.input_collection = 'TriggerPadUpSimHits'
        digi.output_collection= 'trigScintDigisUp'
        return digi

    def down() :
        """Get the digitizer for the trigger pad downstream of target"""
        digi = TrigScintDigiProducer( 'trigScintDigisDn' )
        digi.input_collection = 'TriggerPadDownSimHits'
        digi.output_collection= 'trigScintDigisDn'
        return digi

    def tagger() :
        """Get the digitizer for the trigger pad upstream of tagger"""
        digi = TrigScintDigiProducer( 'trigScintDigisTag' )
        digi.input_collection = 'TriggerPadTaggerSimHits'
        digi.output_collection= 'trigScintDigisTag'
        return digi

