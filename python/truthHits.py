"""Configuration for truth hit producer 

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.TrigScint.truthHits import TruthHitProducer
    p.sequence.extend([ TruthHitProducer.tagger(), TruthHitProducer.up(), TruthHitProducer.down() ])

"""

from LDMX.Framework import ldmxcfg

class TruthHitProducer(ldmxcfg.Producer) :
    """Configuration for truth hit selection producer"""

    def __init__(self,name) :
        super().__init__(name,'trigscint::TruthHitProducer','TrigScint') 

        self.input_collection="TriggerPadUpSimHits"
        self.input_pass_name="" #take any pass
        self.output_collection="truthBeamElectronsUp"
        self.verbose = False

    def up() :
        """Get the beam electron truth hits for the trigger pad upstream of target"""
        trHit = TruthHitProducer( 'truthBeamElectronsUp' )
        trHit.input_collection = 'TriggerPadUpSimHits'
        trHit.output_collection= 'truthBeamElectronsUp'
        return trHit

    def down() :
        """Get the beam electron truth hits for the trigger pad downstream of target"""
        trHit = TruthHitProducer( 'truthBeamElectronsDn' )
        trHit.input_collection = 'TriggerPadDownSimHits'
        trHit.output_collection= 'truthBeamElectronsDn'
        return trHit

    def tagger() :
        """Get the beam electron truth hits for the trigger pad upstream of tagger"""
        trHit = TruthHitProducer( 'truthBeamElectronsTag' )
        trHit.input_collection = 'TriggerPadTaggerSimHits'
        trHit.output_collection= 'truthBeamElectronsTag'
        return trHit

