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
        self.sim_particles_pass_name = ""

    def up() :
        """Get the beam electron truth hits for the trigger pad upstream of target"""
        tr_hit = TruthHitProducer( 'truthBeamElectronsUp' )
        tr_hit.input_collection = 'TriggerPadUpSimHits'
        tr_hit.output_collection= 'truthBeamElectronsUp'
        return tr_hit

    def down() :
        """Get the beam electron truth hits for the trigger pad downstream of target"""
        tr_hit = TruthHitProducer( 'truthBeamElectronsDn' )
        tr_hit.input_collection = 'TriggerPadDownSimHits'
        tr_hit.output_collection= 'truthBeamElectronsDn'
        return tr_hit

    def tagger() :
        """Get the beam electron truth hits for the trigger pad upstream of tagger"""
        tr_hit = TruthHitProducer( 'truthBeamElectronsTag' )
        tr_hit.input_collection = 'TriggerPadTaggerSimHits'
        tr_hit.output_collection= 'truthBeamElectronsTag'
        return tr_hit

