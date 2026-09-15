"""Configuration for truth hit producer

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.TrigScint.truthHits import TruthHitProducer
    p.sequence.extend([
        TruthHitProducer.tagger(), TruthHitProducer.up(), TruthHitProducer.down()
    ])

"""

from LDMX.Framework import Processor, processor


@processor("trigscint::TruthHitProducer", "TrigScint")
class TruthHitProducer(Processor):
    """Configuration for truth hit selection producer"""

    input_collection: str = "TriggerPadUpSimHits"
    input_pass_name: str = ""
    output_collection: str = "truthBeamElectronsUp"
    sim_particles_pass_name: str = ""

    @staticmethod
    def up(**kwargs):
        """Get the beam electron truth hits for the trigger pad upstream of target"""
        return TruthHitProducer(
            instance_name="truthBeamElectronsUp",
            input_collection="TriggerPadUpSimHits",
            output_collection="truthBeamElectronsUp",
            **kwargs,
        )

    @staticmethod
    def down(**kwargs):
        """Get the beam electron truth hits for the trigger pad downstream of target"""
        return TruthHitProducer(
            instance_name="truthBeamElectronsDn",
            input_collection="TriggerPadDownSimHits",
            output_collection="truthBeamElectronsDn",
            **kwargs,
        )

    @staticmethod
    def tagger(**kwargs):
        """Get the beam electron truth hits for the trigger pad upstream of tagger"""
        return TruthHitProducer(
            instance_name="truthBeamElectronsTag",
            input_collection="TriggerPadTaggerSimHits",
            output_collection="truthBeamElectronsTag",
            **kwargs,
        )
