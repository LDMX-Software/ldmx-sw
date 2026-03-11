"""Configuration for ElectronCounter

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.Recon.electron_counter import electron_counter
    p.sequence.append( electron_counter )
"""

from LDMX.Framework import Processor, processor


@processor("recon::ElectronCounter", "Recon")
class ElectronCounter(Processor):
    """Configuration for the event beam electron counter"""

    simulated_electron_number: int
    input_collection: str = "TriggerPadTracksY"
    input_pass_name: str = "truth"
    output_collection: str = "BeamElectronCount"
    use_simulated_electron_number: bool = False


electron_counter = ElectronCounter(
    simulated_electron_number=1,
    instance_name="ElectronCounter",
)
